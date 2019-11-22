#include <glib.h>
//#include <gst/gst.h>
#include <syslog.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include <algorithm>
#include <vector>


#include <unistd.h>

#include <signal.h>
#include <string>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>

#ifdef __cplusplus
#define PROTOBUF_C_BEGIN_DECLS	extern "C" {
#define PROTOBUF_C_END_DECLS	}
#else
#define PROTOBUF_C_BEGIN_DECLS
#define PROTOBUF_C_END_DECLS
#endif

PROTOBUF_C_BEGIN_DECLS
#include <google/protobuf-c/protobuf-c-rpc.h>
PROTOBUF_C_END_DECLS

#include "tracker.pb-c.h"


#include <json/reader.h>
#include <json/value.h>

#define REAL_BOARD

#define RPC_SOCKET_AUDIO "/var/run/rpc-audio.socket"

#define NUM_CHANNELS 16

#define GPIO_AUDIO_RECORD_LED 	(42)
#define GPIO_ADC_POWER			(47)

enum AUDIO_FORMAT
{
	WAV=0,
	MP3=1,
	ALAC=2
};

const char* KEY_CHANNELS="channels";
const char* KEY_AUDIO_FOLDER="audioFolder";
const char* KEY_RECORD_MODE="recordMode";
const char* KEY_SAMPLING_RATE="samplingRate";
const char* KEY_AUDIO_FORMAT="audioFormat";

static pthread_mutex_t rpc_mutex = PTHREAD_MUTEX_INITIALIZER;
pid_t arecord_pid;

FILE *wav_file[NUM_CHANNELS];

GMainLoop *audio_loop;
volatile int is_capture = FALSE;
volatile unsigned int is_encoding = 0;
std::string audio_file_name[NUM_CHANNELS];
static bool ActiveChannels[NUM_CHANNELS];

std::string json_text = "";

GThread *capture_thread;
GThread *encode_thread;

enum RECORD_MODE
{
	SPLITTED=0,
	COMBINED=1
};

static struct
{
	std::string AudioFolder;
	std::string SamplingRate;
	RECORD_MODE RecordMode;
	unsigned ActiveChannelCount;
	unsigned HighestEnabledChannel;
	unsigned ChannelsToRecord;
	AUDIO_FORMAT AudioFormat;
}Settings;

static void SetGpioValue(unsigned nGpio, bool bEnable)
{
	int fd;
	char buffer[64];
	sprintf(buffer,"/sys/class/gpio/gpio%u/value", nGpio);
	fd = open(buffer, O_WRONLY);

	if (-1 == fd) {
		syslog(LOG_ERR,"Unable to set GPIO '%u' to %d", nGpio, bEnable);
	} else {
		sprintf(buffer, "%d", (bEnable?1:0));
		write(fd,buffer,strlen(buffer));
		close(fd);
	}
}

static void AddDirSeperator(std::string& path)
{
	if (path.empty())
		return;
	if (path.at(path.length() -1) != '/') {
		path.append("/");
	}
}

static void LoadDefaultSettings()
{
	Settings.AudioFolder = "/mnt/floppy/audio";
	Settings.SamplingRate = "44100";
	Settings.RecordMode = SPLITTED;
	Settings.ActiveChannelCount = NUM_CHANNELS;
	Settings.HighestEnabledChannel = NUM_CHANNELS;
	Settings.ChannelsToRecord = NUM_CHANNELS;
	AddDirSeperator(Settings.AudioFolder);
	Settings.AudioFormat = WAV;
}

char* IntToString(int iNumber)
{
	static char buffer[32];
	sprintf(buffer,"%d", iNumber);
	return buffer;
}

std::string RemoveExtension(const std::string filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos)
		return filename;
    return filename.substr(0, lastdot); 
}



// Get current date/time, format is YYYY-MM-DD.HH:mm:ss
const std::string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y_%m_%d_%H%M%S", &tstruct);

	return buf;
}

struct MatchPathSeparator
{
	bool operator()( char ch ) const
	{
		return ch == '/';
	}
};

	std::string
basename( std::string const& pathname )
{
	return std::string(
			std::find_if( pathname.rbegin(), pathname.rend(),
				MatchPathSeparator() ).base(),
			pathname.end() );
}


int parse_input(const gchar *input)
{
	int i = 0;

	unsigned active_num_channels = 0;
	unsigned max_channel_num = 0;

	syslog(LOG_INFO, "parsing input: %s", input);

	for (i = 0; i < NUM_CHANNELS; ++i){
		audio_file_name[i] = "";
		ActiveChannels[i] = false;
	}

	Json::Value root;   // will contains the root value after parsing.
	Json::Reader reader;


	if (reader.parse(input, root)) {
		Json::Value temp = root.get(KEY_CHANNELS, Json::Value(Json::arrayValue));
		std::string tempStr;

		for (unsigned i = 0; i < temp.size(); i++){
			Json::Value channel = temp[i].get("num",9999);
			unsigned channel_value;

			if (Json::intValue != channel.type()){
				syslog(LOG_ERR, "value is not an integer\n");
				g_error("value is not an integer\n");
				return -1;

			}

			channel_value = channel.asInt();
			if (channel_value && channel_value <=  NUM_CHANNELS) {
				ActiveChannels[channel_value-1] = true;
				syslog(LOG_INFO, "channel = %d, enabled for capture/encode", channel_value);
				++active_num_channels;
				if (channel_value > max_channel_num) {
					max_channel_num = channel_value;
				}
			}

		}
		Settings.ActiveChannelCount = active_num_channels;
		Settings.HighestEnabledChannel = max_channel_num;
		Settings.ChannelsToRecord = max_channel_num;
		
		if (Settings.ChannelsToRecord) {
			if (Settings.ChannelsToRecord % 2) {
				Settings.ChannelsToRecord++;
			}
		}

		tempStr = root.get(KEY_RECORD_MODE, "").asString();
		if (!tempStr.empty()) {
			Settings.RecordMode = (RECORD_MODE) atoi(tempStr.c_str());
		}

		tempStr = root.get(KEY_SAMPLING_RATE, "").asString();
		if (!tempStr.empty()) {
			Settings.SamplingRate = tempStr;
		}

		tempStr = root.get(KEY_AUDIO_FOLDER, "").asString();
		if (!tempStr.empty()) {
			Settings.AudioFolder = tempStr;
			AddDirSeperator(Settings.AudioFolder);
		}

		Settings.AudioFormat = WAV;
		tempStr = root.get(KEY_AUDIO_FORMAT, "").asString();
		if (!tempStr.empty()) {
			if (tempStr == "mp3") {
				Settings.AudioFormat = MP3;
			} else if (tempStr == "alac") {
				Settings.AudioFormat = ALAC;
			}
		}

	} else {
		syslog(LOG_ERR,"ERROR: can't parse channels state [%s]", input);
		return -1;
	}


	json_text.assign(input);

	return 0;	
}
/*
   void gst_log_function_syslog (GstDebugCategory *category, GstDebugLevel level,
   const gchar *file, const gchar *function,
   gint line, GObject *object,
   GstDebugMessage *message, gpointer data)
   {
   switch (level)
   {
   case GST_LEVEL_ERROR:
   syslog(LOG_ERR, "Error: file:%s, function:%s, line:%d, message:%d", file, function, gst_debug_message_get(message));
   break;
   case GST_LEVEL_WARNING:
   syslog(LOG_WARNING, "Error: file:%s, function:%s, line:%d, message:%d", file, function, gst_debug_message_get(message));
   break;
   case GST_LEVEL_INFO:
   syslog(LOG_INFO, "Error: file:%s, function:%s, line:%d, message:%d", file, function, gst_debug_message_get(message));
   break;
   default:
   break;
   }
   }

   gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
   {
   GMainLoop *loop = (GMainLoop *) data;

   switch (GST_MESSAGE_TYPE (msg)){
   case GST_MESSAGE_EOS:
   syslog (LOG_ERR, "End of stream\n");
   g_main_loop_quit (loop);
   break;
   case GST_MESSAGE_ERROR:
   gchar *debug;
   GError *error;

   gst_message_parse_error (msg, &error, &debug);
   g_free (debug);

   syslog (LOG_ERR, "Error: %s\n", error->message);
   g_error_free (error);

   g_main_loop_quit (loop);

   break;
   default:
   break;
   }

   return TRUE;
   }
 */
void write_little_endian(unsigned int word, int num_bytes, FILE *wav_file)
{
	unsigned buf;
	while(num_bytes>0)
	{   buf = word & 0xff;
		fwrite(&buf, 1,1, wav_file);
		num_bytes--;
		word >>= 8;
	}
}

void write_wav_header(FILE *wav_file, unsigned long num_samples, int s_rate)
{
	unsigned int sample_rate;
	unsigned int num_channels;
	unsigned int bytes_per_sample;
	unsigned int byte_rate;

	num_channels = 1;   /* monoaural */
	bytes_per_sample = 4; // 32 bit's per sample

	if (s_rate<=0) sample_rate = 44100;
	else sample_rate = (unsigned int) s_rate;

	byte_rate = sample_rate*num_channels*bytes_per_sample;

	/* write RIFF header */
	fwrite("RIFF", 1, 4, wav_file);
	write_little_endian(36 + bytes_per_sample* num_samples*num_channels, 4, wav_file);
	fwrite("WAVE", 1, 4, wav_file);

	/* write fmt  subchunk */
	fwrite("fmt ", 1, 4, wav_file);
	write_little_endian(16, 4, wav_file);   /* SubChunk1Size is 16 */
	write_little_endian(1, 2, wav_file);    /* PCM is format 1 */
	write_little_endian(num_channels, 2, wav_file);
	write_little_endian(sample_rate, 4, wav_file);
	write_little_endian(byte_rate, 4, wav_file);
	write_little_endian(num_channels*bytes_per_sample, 2, wav_file);  // block align
	write_little_endian(8*bytes_per_sample, 2, wav_file);  // bits/sample
}

void capture_thread_func(void *ptr)
{
	/*
	// write data subchunk
	fwrite("data", 1, 4, wav_file);
	write_little_endian(bytes_per_sample* num_samples*num_channels, 4, wav_file);
	for (i=0; i< num_samples; i++)
	{   write_little_endian((unsigned int)(data[i]),bytes_per_sample, wav_file);
	}
	 */
	arecord_pid = fork();

	if ( (pid_t)-1 == arecord_pid)
		syslog(LOG_ERR, "fork failed");
	else if ( (pid_t)0 == arecord_pid) {
		char * exec_args[50];
		int arg_count = 0;
		std::vector<std::string> theArgs;

		SetGpioValue(GPIO_ADC_POWER, true);
		SetGpioValue(GPIO_AUDIO_RECORD_LED, true);

		theArgs.push_back("-D");
		theArgs.push_back("plughw:1,0");
		
		theArgs.push_back("-c");
		theArgs.push_back(IntToString(Settings.ChannelsToRecord));

		theArgs.push_back("-r");
		theArgs.push_back(Settings.SamplingRate);
		
		theArgs.push_back("-f");
		theArgs.push_back("S16_LE");

		if (COMBINED == Settings.RecordMode) {
			theArgs.push_back(audio_file_name[0]);
		} else {
			theArgs.push_back("-I");
			for (unsigned i = 0; i < Settings.ChannelsToRecord; ++i)
				theArgs.push_back(RemoveExtension(audio_file_name[i]));
		}

		exec_args[arg_count++] = (char *) "/usr/bin/arecord";
		for (unsigned x = 0; x < theArgs.size(); x++) {
			exec_args[arg_count++] = (char *)theArgs[x].c_str();
		}
		exec_args[arg_count++] = 0; // tell it when to stop!

		execv("/usr/bin/arecord", exec_args);
	} 
	else {
		char cmd[32];
		syslog(LOG_INFO, "audio forked capture process");
		int status = 0;
		sprintf(cmd, "renice -n -10 -p %u", arecord_pid);
		system(cmd);

		//Wait arecord termination
		pid_t w1 = waitpid(arecord_pid, &status, 0);
		if (-1 == w1)
			syslog(LOG_ERR, "fail waitpid on arecord");

		SetGpioValue(GPIO_ADC_POWER, false);
		SetGpioValue(GPIO_AUDIO_RECORD_LED, false);
	}
}
/*
   void capture_gstreamer_thread_func(void *ptr)
   {
   audio_loop = g_main_loop_new (NULL, FALSE);
   GstElement *audio_pipeline = gst_pipeline_new (NULL);
   g_assert (audio_pipeline);

   GstElement *audio_src = gst_element_factory_make("alsasrc", "alsasrc");
   g_assert(audio_src);

   g_object_set(audio_src, "device-name", "plughw:1,0", NULL);
   g_object_set(audio_src, "buffer-time", (guint64)500000, NULL);
   g_object_set(audio_src, "blocksize", 2048, NULL);

   GstCaps *caps = gst_caps_from_string("audio/x-raw-int, endianess=(int)1234, signed=(boolean)true, width=(int)32, depth=(int)32, format=(string)S32LE,rate=(int)44100,channels=(int)16");
   GstElement *caps_filter = gst_element_factory_make ("capsfilter","capsfilter");
   g_assert(caps_filter);
   g_object_set(caps_filter, "caps", caps, NULL);
   gst_caps_unref(caps);

   GstElement *deinterleaver = gst_element_factory_make("deinterleave", "deinterleave");

   GstElement *filesink[NUM_CHANNELS];

   for (int i = 0; i < NUM_CHANNELS; ++i) {
   if ( NONE != audio_file_codec[i] ) {
   char str[32];
   sprintf(str, "filesink%d", i);
   filesink[i] = gst_element_factory_make("filesink", str);

   sprintf(str, "ch%d-", i);
   audio_file_name[i] = str;
   audio_file_name[i] += currentDateTime();

   g_object_set(filesink[i], "location", audio_file_name[i], NULL);
   }
   }

   GstBus *audio_bus = gst_pipeline_get_bus (GST_PIPELINE (audio_pipeline));
   guint audio_bus_watch_id = gst_bus_add_watch (audio_bus, bus_call, audio_loop);
   gst_object_unref (audio_bus);

   gst_bin_add_many (GST_BIN (audio_pipeline), audio_src, caps_filter, deinterleaver, NULL);

   for (int i = 0; i < NUM_CHANNELS; ++i)
   if ( NONE != audio_file_codec[i] )
   gst_bin_add(GST_BIN (audio_pipeline), filesink[i]);

   if (!gst_element_link_many (audio_src, caps_filter, deinterleaver, NULL))
   GST_ERROR ("Failed to link audio_src, caps_filter, deinterleaver");

   for (int i = 0; i < NUM_CHANNELS; ++i){
   if ( NONE != audio_file_codec[i] )
   if (!gst_element_link(deinterleaver, filesink[i]))
   GST_ERROR ("Failed to link deinterleaver and filesink");
   }

// pipeline -> to playing
GST_INFO("starting capture pipeline");
gst_element_set_state (audio_pipeline, GST_STATE_PLAYING);

if (gst_element_get_state (audio_pipeline, NULL, NULL, -1) == GST_STATE_CHANGE_FAILURE)
GST_ERROR("Failed to go into PLAYING state");

g_main_loop_run (audio_loop);

GST_INFO("stopping capture pipeline\n");
gst_element_set_state (audio_pipeline, GST_STATE_NULL);
}
 */
bool start_capture()
{
	char str[16];

	if (!Settings.ActiveChannelCount)
		return false;

	if (is_capture)
		return false;


	is_capture = TRUE;
	std::string date_str = currentDateTime();
	if (COMBINED == Settings.RecordMode) {
		audio_file_name[0] = Settings.AudioFolder;
		audio_file_name[0] += "combined_";
		audio_file_name[0] += date_str;
		audio_file_name[0] += ".wav";
	} else {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			sprintf(str, "ch%d-", i + 1);
			audio_file_name[i] = Settings.AudioFolder;
			audio_file_name[i] += str;
			audio_file_name[i] += date_str;
			audio_file_name[i] += ".wav";

		}
	}

#ifdef REAL_BOARD
	syslog(LOG_INFO,"real capture");

	GError *err1 = NULL ;

	if( (capture_thread = g_thread_create((GThreadFunc)capture_thread_func, NULL, TRUE, &err1)) == NULL)
	{
		syslog(LOG_ERR, "Thread create failed: %s\n", err1->message);
		g_error_free ( err1 ) ;
		return false;
	}
	return true;
#else
#endif
}

void stop_capture()
{
	syslog(LOG_INFO, "stop_capture...");
	is_capture = FALSE;
#ifdef REAL_BOARD
	/*
	   for (int i = 0; i < NUM_CHANNELS; ++i) {
	   if ( NONE != audio_file_codec[i]) {
	   fclose(wav_file[i]);
	   }
	   }

	   g_main_loop_quit (audio_loop);
	 */
	if (0 != kill(arecord_pid, SIGTERM)) {
		syslog(LOG_ERR, "First kill pid = %d failed, errno=%d", arecord_pid, errno);
	}

	g_thread_join(capture_thread);
#endif
}

void encode_thread_func()
{
	if (!Settings.ActiveChannelCount)
		return;
	
	
	syslog(LOG_INFO, "encoding_thread_func");
	is_encoding = 1;

#ifdef REAL_BOARD
	std::string run_str;
	//Loop over all items that have to be encoded and encode in requried format

	//Preprocess files with ffmpeg first, to add WAV header, because output of arecord in PCM format
	if (COMBINED != Settings.RecordMode) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			if (ActiveChannels[i]){
				run_str = "ffmpeg -f s16le -ar " + Settings.SamplingRate + " -ac 1 -i " + RemoveExtension(audio_file_name[i]) + " -acodec pcm_s16le " + audio_file_name[i] ;

				//run encoding process
				int ret = system (run_str.c_str());
				if (0 == ret) {
					unlink(RemoveExtension(audio_file_name[i]).c_str());
				}
				
				if (WIFSIGNALED(ret) &&
						(WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
					continue;
			} else {
				unlink(RemoveExtension(audio_file_name[i]).c_str());
			}
		}
	}

	if (COMBINED != Settings.RecordMode && WAV != Settings.AudioFormat) {
		for (int i = 0; i < NUM_CHANNELS; ++i) {
			if (ActiveChannels[i]){
				if (ALAC == Settings.AudioFormat){
					run_str = "alacconvert " + audio_file_name[i] + " " + RemoveExtension(audio_file_name[i]) + ".alac";
				} else {
					//MP3
					run_str = "lame " + audio_file_name[i] + " " + RemoveExtension(audio_file_name[i]) + ".mp3";
				}

				//run encoding process
				int ret = system (run_str.c_str());
#if 1
				if (0 == ret) {
					unlink(audio_file_name[i].c_str());
				}
#endif
				if (WIFSIGNALED(ret) &&
						(WTERMSIG(ret) == SIGINT || WTERMSIG(ret) == SIGQUIT))
					continue;
			}
		}
	}
#else

	sleep(20);

	char str[16];

	syslog(LOG_INFO, "encoding_thread_func 1...");

	for (j = 0; j < NUM_CHANNELS; j++) {
		syslog (LOG_INFO, "j = %d", j);
		if ( ActiveChannels[j] ) {
			syslog(LOG_INFO, "encode channel %d", j);

			sprintf(str, "ch%d-", j);
			audio_file_name[j] = Settings.AudioFolder;
			audio_file_name[j] += str;
			audio_file_name[j] += currentDateTime();

			if (MP3 == Settings.AudioFormat)
				audio_file_name[j] += ".mp3";
			else
				audio_file_name[j] += ".alac";

			FILE *test_file = fopen(audio_file_name[j].c_str(), "wb+");
			fprintf(test_file, "test encoding");
			fclose(test_file);
		}
	}
#endif
	syslog(LOG_INFO, "encoding finished");
	is_encoding = 0;
	//g_print("stop encoding done: is_encoding = %d", is_encoding);
}

bool start_encoding()
{
	GError *err1 = NULL ;
	if (true == is_capture) {
		return false;
	}

	if( (encode_thread = g_thread_create((GThreadFunc)encode_thread_func, NULL, TRUE, &err1)) == NULL)
	{
		syslog(LOG_ERR, "Thread create failed: %s\n", err1->message);
		g_error_free ( err1 ) ;
		return false;
	}
	return true;
}

static void rpc_audio_set_params(Tracker__Audio_Service *service,
                     const Tracker__String *input,
                     Tracker__ResponseNone_Closure closure,
                     void *closure_data)
{
	syslog(LOG_INFO, "input->value = %s", input->value);

	if (parse_input(input->value))
		syslog(LOG_ERR, "could not parse input json string");

	closure (NULL, closure_data);
}

static void rpc_audio_get_params(Tracker__Audio_Service *service,
                     const Tracker__RequestNone *input,
                     Tracker__String_Closure closure,
                     void *closure_data)
{
	syslog(LOG_INFO, "get_params");

	Tracker__String audio_params = TRACKER__STRING__INIT;

	audio_params.value =(char *) json_text.c_str();

	syslog(LOG_INFO, "audio_params = %s", audio_params.value); 

	closure (&audio_params, closure_data);

	syslog(LOG_INFO, "get_params done!");
}

static void rpc_audio_start_capture(Tracker__Audio_Service *service,
                        const Tracker__RequestNone *input,
                        Tracker__Status_Closure closure,
                        void *closure_data)
{
	syslog(LOG_INFO, "start_capture");
	Tracker__Status Result = TRACKER__STATUS__INIT;
	pthread_mutex_lock(&rpc_mutex);
	if (start_capture()) {
		Result.status = TRACKER__STATUS_ENUM__SUCCESS;
	}
	pthread_mutex_unlock(&rpc_mutex);
	closure(&Result, closure_data);
}

static void rpc_audio_stop_capture(Tracker__Audio_Service *service,
                       const Tracker__RequestNone *input,
                       Tracker__Status_Closure closure,
                       void *closure_data)

{
	syslog(LOG_INFO, "stop_capture");
	Tracker__Status Result = TRACKER__STATUS__INIT;
	pthread_mutex_lock(&rpc_mutex);
	if (true == is_capture) {
		stop_capture();
		Result.status = TRACKER__STATUS_ENUM__SUCCESS;
	}
	pthread_mutex_unlock(&rpc_mutex);
	closure(&Result, closure_data);
}


static void rpc_audio_toggle_encoding(Tracker__Audio_Service *service,
                          const Tracker__RequestNone *input,
                          Tracker__Status_Closure closure,
                          void *closure_data)
{
	syslog(LOG_INFO, "toggle_encoding");
	Tracker__Status Result = TRACKER__STATUS__INIT;
	pthread_mutex_lock(&rpc_mutex);
	if (start_encoding()) {
		Result.status = TRACKER__STATUS_ENUM__SUCCESS;
	}
	pthread_mutex_unlock(&rpc_mutex);
	closure(&Result, closure_data);
}

static void rpc_audio_get_encoding_status(Tracker__Audio_Service *service,
                              const Tracker__RequestNone *input,
                              Tracker__MessageInt_Closure closure,
                              void *closure_data)
{
	syslog(LOG_INFO, "get_encoding_status\n");
	Tracker__MessageInt encoding_status = TRACKER__MESSAGE_INT__INIT;
	encoding_status.value = is_encoding;
	closure(&encoding_status, closure_data);
}



static Tracker__Audio_Service service_audio = TRACKER__AUDIO__INIT(rpc_audio_);

int main (int argc, char *argv[])
{
	openlog("audio", LOG_PID, LOG_DAEMON);
	syslog(LOG_INFO, "Starting");
	SetGpioValue(GPIO_ADC_POWER, false);
	SetGpioValue(GPIO_AUDIO_RECORD_LED, false);
	
	g_thread_init(NULL);

	if (FALSE == g_thread_supported ()){
		syslog(LOG_ERR, "glib threading not started");
		return -1;
	}
	LoadDefaultSettings();
	//gst_init (&argc, &argv);

	//output only warning and errors
	//gst_debug_set_default_threshold(GST_LEVEL_WARNING);
	//gst_debug_add_log_function(gst_log_function_syslog, NULL);

	ProtobufC_RPC_Server *server = protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_LOCAL, RPC_SOCKET_AUDIO, 
			(ProtobufCService *) & service_audio, NULL);

	syslog (LOG_INFO, "started a server");

	while (1)
		protobuf_c_dispatch_run (protobuf_c_dispatch_default ());	

	return 0;
}

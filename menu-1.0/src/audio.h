#ifndef AUDIO_H
#define AUDIO_H 1


#include <string>
#include <map>

using namespace std;

#define RPC_SOCKET_AUDIO "/var/run/rpc-audio.socket"

int rpc_init();
int rpc_de_init();
void set_audio_params(string in);

string get_audio_params();
map<string,string> get_channels_map();

bool start_capture();
bool stop_capture();
bool toggle_encoding();
int get_encoding_status();
void NotifyAudioParameters();

class ToggleMenuEntry;
void SetupAudioRecordHandler(ToggleMenuEntry *pMenu);
#endif

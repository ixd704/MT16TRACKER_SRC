This file describes the procedure to apply the patches available under Mercurial repository on top of the 
VirtualBox image shared by JamHub.


Before you start:
=================
The scripts available under this repository may delete/alter files under LTIB, LTIB/rpm, LTIB/merge, kernel patches.
So always backup your local changes before executing the scripts under this repository.


Procedure:
==========
In order to minimize human errors, I have created some scripts, that will apply the patches on top of the VirtualBox image.

1. Checkout the latest version of Mercurial repository to the VirtualBox/Build machine.
2. In the root directory of the repository, you can find a script named 'merge.sh', issue the command 'sh merge.sh' for 
	applying the patches available under the repository to the build system.

3. From the ltib dir, execute ./ltib -c and change firmware version for generating the updated firmware.



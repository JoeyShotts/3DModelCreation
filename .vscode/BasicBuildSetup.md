# CIS263
This document serves as referance for some of tyhe codeing portions of CIS 263. Each folder is a different configuration of CPP with some kind of explination.

**Configuring compiler:**
You need to install MinGW and add it to the path.
"gcc --version", "g++ --version" and "gdb --version" should all return a valid version.

You can always compile simple C++ files useing "gcc filename.cpp" For more complicated builds CMAKE and/or MAKE may be required. 
*I don't have experience with either of these yet.

**Configuring environment in VS Code:**
You need to have the Microsoft C/C++ and C/C++ extension pack installed.
You need to configure the launch.json file and task.json files in the .vscode file. The .vscode must be in the highest level of the Folder you have open in VS Code.

**task.json:** configures the build process for the script where the code is compiled. This is not needed if you are compiling the code in the terminal or using MAKE.

**launch.json:** configures the run and debug part of the code. It can be configured to just run the most recent version of the code, or recompile the code first. This is done with the preLaunchTask property, it needs to be set to the label property of the task.json file. It also needs to be set "request": "launch" instead of "request": active. You want it to start the executable and use that for debugging not for it to debug a file that is already built. launch.json should be set to launch and debug the currently open code file, it can also be configured to build and launch a specific file if needed.




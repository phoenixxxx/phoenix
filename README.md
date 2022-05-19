Phoenix (how imaginative...)
Release 1.0

## OVERVIEW
This is a general purpose 3D visualizer tool (Windows + D3D11)

## VSCode workflow
- Install C/C++ extension
- Install CMake tools
- Configure the project (automated)
- Debugging
    - Use Windows debugging
    - Use the following `launch.json`
        ```
        {
            // Use IntelliSense to learn about possible attributes.
            // Hover to view descriptions of existing attributes.
            // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
            "version": "0.2.0",
            "configurations": [
                {
                    "name": "Debug C/C++",
                    "type": "cppvsdbg",
                    "request": "launch",
                    "program": "${workspaceFolder}/build/Debug/Phoenix.exe",
                    "externalConsole": true,
                },
            ],
        }
        ```
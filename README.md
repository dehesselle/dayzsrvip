# DayZ Server IP - TS3 plugin
![alt-text](/doc/png/main_window.png)
## About
This plugin can show and automatically distribute and update the IP and name of the server that you're playing on across all your teammates in your channel. No more need to type the IP into the chat yourself or to join via Steam's friendlist.

## Features
>TODO documentation

There is a small [manual](doc/MANUAL.md) that'll get you up and running in a minute!

## Download
The plugin itself is just one file named `dayzsrvip.dll`, available via the [releases](https://github.com/dehesselle/dayzsrvip/releases) page. You need the latest 64bit version of [TeamSpeak](http://www.teamspeak.com) (at the time of writing: 3.0.19.4, plugin API v20) as well as the official [VC++ Redistributable 2015](https://www.microsoft.com/en-US/download/details.aspx?id=48145) (`vc_redist.x64.exe`) from Microsoft.

## Known Issues
When building and replacing the `dayzsrvip.dll` frequently (well it happens during development/testing) I've had my TeamSpeak crash on me a lot. Once it has crashed, everything runs flawlessly until the next time I replace the `.DLL` with a newer build, so I'm hoping that's "developer only" issue.
>TODO documentation

## Build
>TODO documentation

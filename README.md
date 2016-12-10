# DayZ Server IP - TS3 plugin
![alt-text](doc/png/main_window.png)
## About
This plugin can show and automatically distribute and update the IP and name of the server that you're playing on across all your teammates in your channel. No more need to type the IP into the chat yourself or to join via Steam's friendlist.

## Features
- Whenever you join a server, a message to your current channel is sent. The plugin picks up on the message and updates the "player list" section (treeview in the lower half of the GUI).
- If you don't like or need the GUI, just close it. The plugin will keep running.
- The data in the "player list" section is saved to disk.

## Usage
There is a [manual](doc/MANUAL.md) that'll get you up and running in a minute!

## Download

The plugin itself is just one file named `dayzsrvip.dll`, packaged as `.ts3_plugin` and available on the [releases](https://github.com/dehesselle/dayzsrvip/releases) page. If you prefer the plain DLL, rename the `.ts3_plugin` to `.zip` and get it from there.

#### Requirements
Starting with v0.5, you need a 64bit version of TeamSpeak 3.1.x that provides plugin API v21. For [TeamSpeak](http://www.teamspeak.com) 3.0.x (API v20), use v0.4 instead.

## Build
Building this yourself is pretty straight forward. Download and install [Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48146) and [Qt 5.6.1](http://download.qt.io/official_releases/qt/5.6/5.6.1/). Open Qt Creator. Open the project `plugin/dayzsrvip.pro`. Start compiling.

## Issues and Limitations
This project is still in development and needs testing - there will be bugs!  
There are only very few safeguards in place, so if you intentionally want to break stuff, you will succeed.

## Credits
- [TeamSpeak](http://http://www.teamspeak.com)
- [Visual Studio](https://www.visualstudio.com)
- [Qt](https://www.qt.io)

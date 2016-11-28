# DayZ Server IP - TS3 plugin
![alt-text](doc/png/main_window.png)
## About
This plugin can show and automatically distribute and update the IP and name of the server that you're playing on across all your teammates in your channel. No more need to type the IP into the chat yourself or to join via Steam's friendlist.

## Features
>TODO documentation

## Usage
There is a [manual](doc/MANUAL.md) that'll get you up and running in a minute!

## Download

The plugin itself is just one file named `dayzsrvip.dll`, packaged as `.ts3_plugin` and available on the [releases](https://github.com/dehesselle/dayzsrvip/releases) page. If you prefer the plain DLL, rename the `.ts3_plugin` to `.zip` and get it from there.
#### Requirements
You need a recent 64bit version of [TeamSpeak](http://www.teamspeak.com) that provides plugin API v20. Just update to the latest version if you're not there already (at the time of writing: 3.0.19.4) and you're good to go.

## Build
Building this yourself is pretty straight forward. Download and install Visual Studio 2013 and Qt 5.5.1. Open Qt Creator. Open the project `plugin/dayzsrvip.pro`. Start compiling.

## Known Issues
This project is still under heavy development and needs testing - there will be bugs!
- documentation is not up to par with current release (sorry - things are happening fast now and fixing bugs for the people who take the time to test is way more important)

## TODOs
- documentation
- code cleanup
- ...

## Credits
- [TeamSpeak](http://http://www.teamspeak.com)
- [Visual Studio](https://www.visualstudio.com)
- [Qt](https://www.qt.io)

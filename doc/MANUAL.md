# TL;DR - quickstart
:information_source: __This is for the impatient ones - the detailed user guide is below!__

So you know your shit and don't want/need me to guide you through?  
Fine by me - let's go!

1. Install the plugin by double-clicking `dayzsrvip.ts3_plugin`. Make sure it's enabled.
2. Open its window via `Show` in the `Plugins`, `DayZ Server IP` menu.
3. Click `Open...` to select your `.DayZProfile` file.
4. Join your TeamSpeak server and channel, then click `on`.
5. Have everybody else who you're going to play with go through steps 1-4.
6. Join a DayZ server.

![alt-text](png/main_window.png)

# DayZ Server IP - user guide

Follow these few steps and you'll be up and running in a few minutes!

Just to make sure there's no misunderstanding: you and all your friends need to install and use this plugin, it doesn't work own its own!
(If it did, that would be real magic!)

## Install plugin
You can choose to install the plugin manually (download the `.zip`) or automatically (download the `.ts3_plugin`).

### automatic installation
Download and double-click `dayzsrvip.ts3_plugin`. TeamSpeak's plugin installer will open and do everything for you.

![alt-text](png/ts3_plugin_installer.png)

### manual installation
Download `dayzsrvip_n.n_x64.zip` (`n.n` for the version number) and extract it.
You need to put the `dayzsrvip.dll` into TeamSpeak's `plugins` directory.

![alt-text](png/explorer_ts3_folder.png)

#### Enable plugin
After putting it in the `plugins` folder, start Teamspeak and go to `Settings`, `Plugins`...

![alt-text](png/ts3_settings_menu.png)

...and enable it.

![alt-text](png/ts3_enable_plugin.png)

If the plugin doesn't show up there, you may need to click the `Reload All` button.

## Open plugin window
After having enabled the plugin it will show up in the `Plugins` menu. Since it runs in its own separate window, click on `DayZ Server IP`, `Show`.

![alt-text](png/ts3_plugins_menu.png)

This is what it looks like on its first start.

![alt-text](png/main_window_init.png)

## Description

Most of the upper half of the window is the "local info" section __(1)__. This is where your TS3 name, in-game name, server name and IP are shown. The lower half is the "remote info" section __(2)__. It shows everybody else's in-game name, server name and server IP along with a timestamp to indicate when the last update was received.

![alt-text](png/main_window_init_annotated.png)

The buttons __(3)__ and __(4)__ are to select your `.DayzProfile` file and to turn this thing on and off. `on` __(4)__ means that your data will be sent to other clients in your TeamSpeak channel. It's disabled until you have chosen a valid `.DayZProfile` with the `Open...` button __(3)__. While the plugin will remember your profile from now on, it'll always start in `off` mode __(4)__ on purpose.

The "remote info" section __(2)__ gets updated automatically whenever somebody joins a server. That also means that there won't be any updates in between, while everyone is playing. So if you're late to the party and everybody else has already joined a server, you'll have missed all those initial updates - and that's not good, is it? That's what the `sitrep` button __(5)__ is there for. You can manually request an update message from all other clients to get you started.

The `Clear` button __(6)__ is hopefully self-explanatory, it gets rid of all the data in the "remote info" __(2)__ section.

The `Debug...` and `Log` buttons __(7)__ are there to help me with debugging.

# How does this work?
It's not magic, that's for sure!

Whenever there are changes to your `.DayZProfile` file, the plugin gathers server IP, server name and in-game name from it and sends a text message to your current channel in TeamSpeak. Those messages are plain text messages (you could even type them in yourself), but have a specific format so that the plugin can pick up on them. Pretty simple, eh? :wink:

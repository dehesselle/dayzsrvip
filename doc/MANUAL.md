# DayZ Server IP - user guide
Follow these few steps and you'll be up and running in a few minutes!

Just to make sure there's no misunderstanding: you and all your friends need to install and use this plugin, it doesn't work own its own!
(If it did, that would be real magic!)

## Install plugin
You need to put the `dayzsrvip.dll` into TeamSpeak's plugin directory.

![alt-text](/doc/png/explorer_ts3_folder.png)

## Enable plugin
After putting it in the `plugins` folder, start Teamspeak and go to `Settings|Plugins`...

![alt-text](/doc/png/ts3_settings_menu.png)

and enable it.

![alt-text](/doc/png/ts3_enable_plugin.png)

If the plugin doesn't show up there, you may need to click the `Reload All` button.

## Open plugin window
After having enabled the plugin it will show up in the `Plugins` menu. Since it runs in its own separate window, click on `Show`.

![alt-text](/doc/png/ts3_plugins_menu.png)

Now you're half way there. This is what it looks like on its first start.

![alt-text](/doc/png/main_window_init.png)

### Description
Most of the upper half of the window is the "local info" section __(1)__. This is where your own ingame name, server name and IP are shown. The lower half is the "remote info" section __(2)__. It shows everybody else's ingame name, server name and server IP along with a timestamp to indicate when the last update was received. If this view gets to cluttered, you can clear it by clicking the `Clear` button __(6)__. (Right now this is only temporary and the complete history will reapper when the plugin is restarted.)

### Usage
You have to click the `Open...` button __(3)__ to select your `.DayZProfile` file that will be used to gather your data. Once you've selected a valid profile, the radiobuttons on the left side __(4)__ become active (well, the `on` button does). If you select `on` the plugin will start to send your data via text messages to the TeamSpeak channel. But since only updates are sent, you can manually request an update by clicking the `sitrep` button __(5)__. (This will probably be automated in the future.)

![alt-text](/doc/png/main_window_annotated.png)

## Now what?
- Click on `Open...` and select your `.DayZProfile` file. The dialog should have already placed you inside of your `Documents/DayZ` folder to ease up the task.
- The placeholders like `___CHAR_NAME___` and the likes will now be replaced by the data that was gathered from your `.DayZProfile` file.
- Join a TeamSpeak server, click the radio button `on` and you'll be distributing your server connection data to the TeamSpeak channel.

If everything works as intended you'll now be able to see your and your teammates data.

![alt-text](/doc/png/main_window.png)

## How does this work?
It's not magic, that's for sure!

Whenever there are changes to your `.DayZProfile` file, the plugin gathers server IP, server name and ingame name from it and sends a text message to your current channel in TeamSpeak. Those messages are plain text messages (you could even type them in yourself), but have a specific format so that the plugin can pick up on them. Pretty simple, eh? ;)

# DioPanelApplets
A simple nice compact panel with basic applets written in GTK.
You can use it independently in any Desktop Environment.
It was tested on Debian 12.

# What you can do with DioPanelApplets
   1. Adjust system volume and brightess.
   2. Adjust size and position of the panel.
   3. Add up to three keyboard layouts.
   4. Take quick notes.
   5. Manage your wifi networks.

# Installation/Usage
  1. Open a terminal and run:

		 ./configure

  2. if all went well then run:

		 make
		 sudo make install
		 
		 (if you just want to test it then run: make run)
		
  4. Run the application or add it to startup:
  
		 diopanelapplets

# Screenshots
 
![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-settings.png)
 
![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-volume.png)

![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-brightness.png)

![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-clock.png)

![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-notes.png)

![Alt text](https://github.com/DiogenesN/diopanelapplets/blob/main/diopanelapplets-network.png)

# BUGS

When you connect to internet and type in your password, no issues if you enter the correct password, but if you enter a wrong password then the application might freeze, to fix it you need to open: nm-connection-editor, remove your network name from the saved networks in 'Wi-Fi section, disable and re-enable Wifi and connect again.

That's it!

 Make sure you have the following packages installed:

		sed
		gawk
		make
		pkgconf
		libgtk-4-dev
		x11-xkb-utils
		network-manager
		pulseaudio-utils
		x11-xserver-utils

# Support

   My Libera IRC support channel: #linuxfriends
   
   Email: nicolas.dio@protonmail.com


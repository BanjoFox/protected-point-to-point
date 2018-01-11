               Installing P3 Packages

These are the installation instructions for Velocite Systems P3 package on a
target system.  There are three packages for different distributions of Linux,
and one for the gingerbread Android system.  If a specific distribution is
required and it is not included in this group, contact Velocite Systems.


RUNTIME REQUIREMENT LISt

    Distribution    Kernel level
	------------    ------------
	Redhat          2.6.18-274.7.1.el5
	CentOS          2.6.18-274.7.1.el5
	Ubuntu          2.6.31-23-generic
    Android         2.6.35.7-59465-g42bad32


INSTALLATION FILES

The installation files for each distribution is in the directory by that name.
Each package includes the required P3 files and an installation script for
the target platform.


INSTALLATION ON LINUX

The instructions for installing Velocite Systems P3 on Linux are:

  1) Login as root

    su

  2) Change to the appropriate directory and edit the appropriate
     configuration file See the P3 Functional Description Appendix A
     for help

	cd [distro_name]
    <edit> p3primary.conf
	  OR
    <edit> p3secondary.conf

  3) Run the installation script

	./install.sh


INSTALLATION ON ANDROID

NOTE: The phone MUST be rooted for these instructions to work.
  See the RootingAndroid directory for helpful information on rooting
  your phone

Enable USB debugging on the phone and connect your it to the computer. 
- You can enable USB Debugging by going to:
    Settings > Applications > Development
  and checkmark USB Debugging option.

The instructions for installing Velocite Systems P3 on Android are:

  1) Change to the Android directory and edit the configuration file
     See the P3 Functional Description Appendix A for help

	cd Android
    <edit> p3secondary.conf

  2) Run the installation script

	./install.sh


DOCUMENTATION

The P3 Functional Description is a PDF file. This contains an appendix with
information on configuring P3.

Error messages for the system application are written to the P3 log file,
which is located in /var/log/p3 by default (/data/local/tmp on Android).
Error messages for the kernel module are written to the kernel log and
can be displayed using the 'dmesg' command.

Error messages are documented with the message text, a description of why the
message was issued, and the appropriate response to the message.  The
description of error messages are contained in the html files included in
this package as follows:

  P3SYSTEM_MSGS.html:  System application messages
  P3KM_MSGS.html:  Kernel module messages


RUNNING P3

Currently P3 must be started manually.  An init script may be created by the
superuser to automate this a boot time.

  To start the P3 system, login as root and run the P3 startup script

    p3run p

	  OR

    p3run s

	  OR (Android only)

    adb/adb-linux shell
	su
	cd /data/local/tmp
	./p3run

  To stop the P3 system, login as root and run the P3 startup script

    p3stop p

	  OR

    p3stop s

	  OR (Android only)

    adb/adb-linux shell
	su
	cd /data/local/tmp
	./p3stop


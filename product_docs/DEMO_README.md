          Running a P3 demo

Perform the following steps in the order specified:

1) Login to the P3 primary and secondary hosts and any hosts to be
   used for transferring data.

2) Login to the 'demo' user and cd to the demo directory in its
   home directory.

3) On the P3 secondary, run the script 'runsec.sh'.

   NOTE: At this point, DO NOT type anything in a terminal window
      logged into a secondary host.  This includes the P3
      secondary and any host in the secondary network.

4) On the P3 primary, run the script 'runpri.sh'.  When it is complete,
   press Enter in the P3 secondary terminal window.

   NOTE: At this point, it is safe to type in a terminal window
      logged into a secondary host.

5) Perform the file transfers for the demo.

6) On the P3 secondary, run the script 'stopsec.sh'.

   NOTE: At this point, DO NOT type anything in a terminal window
      logged into a secondary host.  This includes the P3
      secondary and any host in the secondary network.

7) On the P3 primary, run the script 'stoppri.sh'.  When it is complete,
   press Enter in the P3 secondary terminal window.

   NOTE: At this point, it is safe to type in a terminal window
      logged into a secondary host.

8) On the P3 primary, there will be 2 formatted files created from
   the tcpdump data:
   - netcat.trace:  This is the tcpdump output and only displays the
     encrypted data.
   - netcat.p3:  This is the formatted P3 headers and decrypted payload,
     showing the headers and data.



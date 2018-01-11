# Protected Point-to-Point (P3), and Secure Storage Solution (S3)

## Project Background
I co-invented a VPN-like method for securing transmissions where encryption keys could be changed without having to tear-down, and rebuild the entire session. At the time we also tried to patent the technology but essentially ran out of funding. The organization has since dissolved. This was back in 2010. Given that the invention always sparked interest, but never truly gone anywhere I would like to begin the process of opening it up to a wider audience.

What follows are a few of the high-level design elements which we thought were unique to the art. 

- Dynamic key changes during encrypted sessions (no need to restart)
- Initial key exchange agnostic. Ex: Possible to use IPSec/DH exchange to build initial tunnel
- Packets containing keys are obfuscated using upto three methods
- Can also utilize locally stored key arrays. Sending index of key instead of key itself.

We were able to come up with working code (Linux 2.6 kernel & Android Gingerbread), which I can share once I remove the proprietary crypto libraries. I also have all of the documentation/presentation slides that I authored at the time.


## The Protected Point-to-Point solution provides a continuously secure channel for data between connection points 
- Encryption keys are updated without closing the connection
- Keys can be changed frequently without detection by an outside observer
- data sessions to proceed uninterrupted by a re-keying process
- Eliminates human administration of keying operations

## The Secure Storage Solution 
 - encrypts data at a file level on any hard drive device
 - maintains the encryption key separate from the data
 - assigns a unique key to each encrypted file
 - This prevents any stolen data from being decrypted
 
## Directory Structure
 - Android-P3: This directory has Android binaries, and other tools that were used for the live demonstrations. Possibly redundant.
 - Binaries: This has working binaries for Android, CentOS, Redhat, Ubuntu. The RedHat_PrimaryPlus also has binaries for a PrimaryPlus setup.
 - doc: These are the Doxygen files for the source code
 - product_docs: Documents, and demonstration videos, includes specificaions
 - src: Source code

## Technical details
P3 works by building an encrypted tunnel on top of another tunnel.  The "interior" tunnel, called a 'control channel' is used to pass new keys/key-related information, to the other endpoint(s).
The "exterior" tunnel, called a 'data channel' is used for passing actual network traffic. There are some caveats though.

- The initial handshake still needs to be done by something else (we used IPSec)
- We also used a proprietary crypto library (not included), although we were thinking about having an OpenSSL port.


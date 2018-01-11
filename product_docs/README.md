The P3 configuration files assume the following network configuration:

P3 Primary Network:

- Subnet = 192.168.2.0, Mask = 255.255.255.0
- P3 Primary Host = 192.168.2.1

P3 Secondary Network:

- Subnet = 192.168.3.0, Mask = 255.255.255.0
- P3 Secondary Host = 192.168.3.1

The following instructions assume that you are in the same directory as
the programs and configuration files.

To run this on the primary, enter the following commands:

sudo insmod p3primary.ko
sudo ./p3primary -h ./ -f pcfg


To run this on the secondary, enter the following commands:

sudo insmod p3secondary.ko
sudo ./p3secondary -h ./ -f scfg

It should be possible to send data from a host in either network to the
other network.  However the data should be no larger than 1300 bytes.



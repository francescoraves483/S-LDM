# ms-van3t UDP mode namespace creator script

This folder contains a script which can be used to create (and delete) a new network namespace (named `ns1`), which can be very helpful for setting up the communication between [ms-van3t](https://github.com/marcomali/ms-van3t) (in UDP emulation mode) and the UDP->AMQP relayer in "loopback", i.e. using the same device and on the same OS.

In order to run ms-van3t and make it communicate with the relayer "in loopback", you need to:
- Enable the +x permission on the script (needed only once): `chmod +x ms-van3t-namespace-creator.sh`
- Launch `./ms-van3t-namespace-creator.sh`; then, leave this terminal always open and open another terminal/tab for the other points
- Launch the relayer with the UDP socket waiting for messages from ms-van3t
- Start ms-van3t in UDP emulation mode (inside the `ns1` namespace) with: `sudo ip netns exec ns1 ./waf --run "v2x-emulator --interface=veth1ns --subnet=10.10.7.0 --gateway=10.10.7.254 --udp=10.10.7.254:20000 --sumo-netns=ns1"`
- When you finish testing and you want to delete the namespace (undoing the modifications applied by the script), simply open the terminal which was left open, with the script, and press ENTER.

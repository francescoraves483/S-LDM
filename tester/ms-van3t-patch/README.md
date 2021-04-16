# S-LDM Tester ms-van3t patch files

This folder contains some patch files and a patch script (`0001-patch-ms-van3t-sldm.patch`) to patch [ms-van3t](https://github.com/marcomali/ms-van3t), in order to make it compatible with the UDP->AMQP 1.0 relayer. 

**Warning:** Using this patch may break compatibility with other applications relying on the emulation capabilities of ms-van3t.

In order to use this patch, copy all the four files inside `ms-van3t/ns-3.33` and run `patch-ms-van3t-sldm.sh`.

This patch modifies the GeoNetworking layer of ms-van3t to send 16 B of additional metadata before the actual ETSI packet (CAM/DENM + BTP + GeoNetworking), containing the current stationID (64 bits), latitude (32 bits - needs to be divided by 1e7 to get the degrees at the receiving program) and longitude (32 bits - needs to be divided by 1e7 to get the degrees at the receiving program) of the vehicle sending that particular message.

This is used by the tester to compute the right Quadkeys to be inserted as AMQP 1.0 message properties.
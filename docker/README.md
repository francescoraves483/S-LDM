# Docker commands for building a Docker container with the S-LDM

In order to build a Docker container containing the S-LDM, you should use, **from the main repository folder**:
```
sudo docker build -t cnitdocker/sldm_image -f docker/Dockerfile .
```

You can then run the container with (sample parameters are shown here - you are then expected to modify them depending on where the S-LDM will be deployed and which area it will cover):
```
sudo docker run -dit \
--env SLDM_INTERNAL_AREA="46.122157:11.074610-46.242090:11.259199" \
--env SLDM_EXTERNAL_AREA_LAT_FACTOR="0.002" \
--env SLDM_EXTERNAL_AREA_LON_FACTOR="0.002" \
--env BROKER_URL="127.0.0.1:5672" \
--env AMQP_TOPIC="topic://5gcarmen.examples" \ 
--env MS_REST_ADDRESS="http://localhost" \
--env MS_REST_PORT="8080" \
--env VEHVIZ_UDP_ADDRESS="127.0.0.1" \
--env VEHVIZ_UDP_PORT="48110" \
--env VEHVIZ_WEB_PORT="8080" \
--net=host --name=sldm_container cnitdocker/sldm_image
```

Or (in a single line):
```
sudo docker run -dit --env SLDM_INTERNAL_AREA="46.122157:11.074610-46.242090:11.259199" --env SLDM_EXTERNAL_AREA_LAT_FACTOR="0.002" --env SLDM_EXTERNAL_AREA_LON_FACTOR="0.002" --env BROKER_URL="127.0.0.1:5672" --env AMQP_TOPIC="topic://5gcarmen.examples" --env MS_REST_ADDRESS="http://localhost" --env MS_REST_PORT="8080" --env VEHVIZ_UDP_ADDRESS="127.0.0.1" --env VEHVIZ_UDP_PORT="48110" --env VEHVIZ_WEB_PORT="8080" --net=host --name=sldm_container cnitdocker/sldm_image
```

The commands reported above will run the S-LDM container and grant it access to the host networking (i.e., everything will work, from the network point of view, as if the S-LDM is run outside the container). This behaviour can be modified by changing the `--net=host` option.

After running the container, it can be started/stopped with `sudo docker container start sldm_container` and `sudo docker container stop sldm_container`.

You can view, instead, the output of the S-LDM with `sudo docker logs sldm_container`.
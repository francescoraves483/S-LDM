FROM ubuntu:20.04

ENV TZ=Europe/Rome
# ENV SLDM_INTERNAL_AREA 46.122157:11.074610-46.242090:11.259199
# ENV SLDM_EXTERNAL_AREA_LAT_FACTOR 0.01
# ENV SLDM_EXTERNAL_AREA_LON_FACTOR 0.01
# ENV BROKER_URL 127.0.0.1:5672
# ENV AMQP_TOPIC topic://5gcarmen.examples
# ENV MS_REST_ADDRESS http://localhost
# ENV MS_REST_PORT 8000
# ENV VEHVIZ_UDP_ADDRESS 127.0.0.1
# ENV VEHVIZ_UDP_PORT 48110
# ENV VEHVIZ_WEB_PORT 8080

# Set timezone
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Create "sldm" user
RUN useradd -ms /bin/bash sldm

# Copy the S-LDM source code inside the container
COPY . /home/sldm/SLDM

# Install dependencies
RUN apt-get update && \
    apt-get install -y apt-utils && \
    apt-get install -y build-essential cmake cmake-curses-gui uuid-dev libssl-dev libsasl2-2 libsasl2-dev libsasl2-modules wget python libcpprest libcpprest-dev nodejs

# Download Qpid Proton
RUN wget https://downloads.apache.org/qpid/proton/0.33.0/qpid-proton-0.33.0.tar.gz && \
    tar zxvf qpid-proton-0.33.0.tar.gz

# Install Qpid Proton
RUN cd qpid-proton-0.33.0 && \
    mkdir build && \
    cd build && \
    cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DSYSINSTALL_BINDINGS=ON -DBUILD_PYTHON=OFF -DBUILD_RUBY=OFF -DBUILD_GO=OFF && \
    make all && \
    make install

# Compile the S-LDM
RUN cd /home/sldm/SLDM && \
    make fullclean && \
    make
    
USER sldm

WORKDIR /home/sldm/SLDM
# EXPOSE 5671 5672 8161

CMD ["/bin/sh", "-c", "./SLDM -A ${SLDM_INTERNAL_AREA} --ext-area-lat-factor ${SLDM_EXTERNAL_AREA_LAT_FACTOR} --ext-area-lon-factor ${SLDM_EXTERNAL_AREA_LON_FACTOR} --broker-url ${BROKER_URL} --broker-queue ${AMQP_TOPIC} --ms-rest-address ${MS_REST_ADDRESS} --ms-rest-port ${MS_REST_PORT} --vehviz-nodejs-address ${VEHVIZ_UDP_ADDRESS} --vehviz-nodejs-port ${VEHVIZ_UDP_PORT} --vehviz-web-port ${VEHVIZ_WEB_PORT}"]

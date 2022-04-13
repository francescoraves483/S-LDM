FROM ubuntu:20.04

ENV TZ=Europe/Rome
# Default area between San Michele all'Adige and Egna (Italy)
ENV SLDM_INTERNAL_AREA 46.174740:11.108462-46.339863:11.312573
ENV SLDM_EXTERNAL_AREA_LAT_FACTOR 0.0002
ENV SLDM_EXTERNAL_AREA_LON_FACTOR 0.0002
# Default broker URL and port
ENV BROKER_URL 127.0.0.1:5672
# Default broker topic
ENV AMQP_TOPIC topic://5gcarmen.examples
ENV MS_REST_ADDRESS http://localhost
ENV MS_REST_PORT 8000
ENV VEHVIZ_UDP_ADDRESS 127.0.0.1
ENV VEHVIZ_UDP_PORT 48110
ENV VEHVIZ_WEB_PORT 8080
ENV LOG_FILE stdout
# To set a default plain authentication with username "user" and password "passwd", use: ENV SLDM_OTHER_OPTIONS "-u user -p passwd -I"

# Set timezone
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Create "sldm" user
RUN useradd -ms /bin/bash sldm

# Copy the S-LDM source code inside the container
COPY . /home/sldm/SLDM

# Install dependencies
RUN apt-get update && \
    apt-get install -y apt-utils && \
    apt-get install -y build-essential cmake cmake-curses-gui uuid-dev libssl-dev libsasl2-2 libsasl2-dev libsasl2-modules wget python3 libcpprest libcpprest-dev nodejs libgeographic-dev

# Download Qpid Proton
RUN wget https://archive.apache.org/dist/qpid/proton/0.35.0/qpid-proton-0.35.0.tar.gz && \
    tar zxvf qpid-proton-0.35.0.tar.gz

# Install Qpid Proton
RUN cd qpid-proton-0.35.0 && \
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

CMD ["/bin/sh", "-c", "./SLDM -A ${SLDM_INTERNAL_AREA} --ext-area-lat-factor ${SLDM_EXTERNAL_AREA_LAT_FACTOR} --ext-area-lon-factor ${SLDM_EXTERNAL_AREA_LON_FACTOR} --broker-url ${BROKER_URL} --broker-queue ${AMQP_TOPIC} --ms-rest-address ${MS_REST_ADDRESS} --ms-rest-port ${MS_REST_PORT} --vehviz-nodejs-address ${VEHVIZ_UDP_ADDRESS} --vehviz-nodejs-port ${VEHVIZ_UDP_PORT} --vehviz-web-port ${VEHVIZ_WEB_PORT} -L ${LOG_FILE} ${SLDM_OTHER_OPTIONS}"]

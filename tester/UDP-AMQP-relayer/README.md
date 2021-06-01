# UDP->AMQP 1.0 relayer

This folder contains the UDP->AMQP 1.0 relayer source file and the Makefile to compile the whole program.

The relayer is intended to work in combination with [ms-van3t](https://github.com/marcomali/ms-van3t), after patching the framework with the files available in `./tester/ms-van3t-patch"`.

The relayer will then receive the CAM messages from ms-van3t, and forward them to the AMQP broker, properly filling in the "quadkeys" property of each message, depending on the current positions of vehicles.

This relayer will **not** work with any unpatched version of ms-van3t.

In order to compile the relayer, you can use the Makefile included in this directory. You can thus compile the relayer executable simply with `make`.
You can then launch the UDP->AMQP relayer with: `./UDPAMQPrelayer`.

If no options are specified, the relayer will try to connect to `127.0.0.1:5672` and use as a default topic name `topic://5gcarmen.examples`.

You can also specify a custom URL or topic/queue name with, respectively, the `--url` and `--queue` options.

This relayer has been tested with an [Apache ActiveMQ "Classic"](https://activemq.apache.org/components/classic/download/) broker (version 5).

The relayer relies on the [TCLAP library](http://tclap.sourceforge.net/) in order to parse the command line options.
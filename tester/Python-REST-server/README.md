# Python mock REST server

This folder contains a Python 3 mock REST server for testing the S-LDM Maneuvering Service REST client.

This simple mock server is configured to bind at `127.0.0.1:8000` (i.e., the default S-LDM options for the REST interface towards the Maneuvering Service) and can be launched with `python3 ./RESTserver.py`.

Then, when a connection is established from the S-LDM, the server is configured to receive up to 10 periodic POST requests, after which a STOP response is sent to the S-LDM.

The STOP response will have the following content:
```
json_response = {
	"rsp_time": <current time>,
	"post_time": <generation timestamp of the POST request>,
	"rsp_type": "STOP"
};
```

For each POST request, a REST interface client->server latency value is printed, together with the content of the JSON data structure received from the S-LDM.

# Pre-requisites

This Python mock REST server requires the `tornado` package to be installed. 

Under Ubuntu, you can install it with `pip3 install tornado`.
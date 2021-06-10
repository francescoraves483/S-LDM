import tornado.web
import tornado.httpserver
import tornado.ioloop
import json
import time

class dataStore:
	def __init__(self,num_posts):
		self.num_posts = num_posts

	def get_num_posts(self):
		return self.num_posts

	def set_num_posts(self,num_posts):
		self.num_posts = num_posts

class sampleserver(tornado.web.RequestHandler):
	def initialize(self,data):
		self.data = data;

	def post(self):
		num_posts = self.data.get_num_posts()

		# New POST data received
		num_posts = num_posts + 1

		# Read the data coming from the POST request
		json_data = json.loads(self.request.body)

		print("POST #" + str(num_posts))
		# print("Received a new JSON with the following stationIDs:")
		# for vehicle in json_data["vehicles"]:
		# 	print(str(vehicle["stationID"])+",")

		now = time.time_ns()

		print("Estimated client->server latency: "+str(now/1e6-json_data["generation_tstamp"]/1e3)+" ms");

		print("JSON content: ");
		print(json.dumps(json_data,indent=2,sort_keys=False));

		# print("--------------")

		json_response = {
			"rsp_time": now,
			"post_time": json_data["generation_tstamp"],
			"rsp_type": "OK"
		};

		if(num_posts>=10):
			num_posts = 0
			json_response["rsp_type"] = "STOP"

		self.data.set_num_posts(num_posts)

		self.write(json_response)

data = dataStore(0)
app=tornado.web.Application([(r'/',sampleserver,dict(data = data))])
server=tornado.httpserver.HTTPServer(app)
try:
	server.bind(8000)
except:
	print("Cannot create connection on port:",http_port)
	exit(1)

try:
    server.start(1)
    tornado.ioloop.IOLoop.current().start()
except KeyboardInterrupt:
    tornado.ioloop.IOLoop.current().stop()
    server.stop()
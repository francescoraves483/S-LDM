import socket
import sys
import argparse
import json
import time

def main():
	parser = argparse.ArgumentParser(description='Sample JSON TCP client for the S-LDM')
	parser.add_argument('--server_ip_address', nargs='?', default="127.0.0.1", help='S-LDM JSON server address')
	parser.add_argument('--server_port', nargs='?', default=49000, help='S-LDM JSON server port')
	parser.add_argument('--radius_m', nargs='?', default=0, help='Radius (in meters) of the area around the center latitude and longitude from which the data should be gathered')
	parser.add_argument('latitude', help='Center latitude around which the data should be gathered')
	parser.add_argument('longitude', help='Center longitude around which the data should be gathered')
	args = parser.parse_args()

	# Create a new TCP socket
	tcp_sock=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	
	# Connect to the server (i.e., to the S-LDM)
	tcp_sock.connect((args.server_ip_address, int(args.server_port)))
	
	welcome_msg=tcp_sock.recv(1024);
	
	if(welcome_msg.decode("utf-8")=="Connection: confirmed"):
		if float(args.radius_m)>0:
			request={"lat": float(args.latitude), "lon": float(args.longitude), "range": float(args.radius_m)}
		else:
			request={"lat": float(args.latitude), "lon": float(args.longitude)}
		
		tcp_sock.sendall(bytes(json.dumps(request)+"\0",encoding="utf-8"))
		
		# Recive and parse reply
		rx_data_str=tcp_sock.recv(4096).decode("utf-8")
		
		print(rx_data_str);
		
		rx_data=json.loads(rx_data_str)
		
		print("Received JSON file:")
		print(json.dumps(rx_data,indent=2,sort_keys=False));
	else:
		print("Error: received a wrong confirmation message:",welcome_msg.decode("utf-8"));
		sys.exit(1);

if __name__ == "__main__":
	main();

#!/usr/bin/python
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
from os import curdir, sep
from subprocess import Popen

PORT_NUMBER = 8899
commands = {
	"/click" : "echo wibble",
	"/ocfon" : "cd ../ocfControl && ./ocfSwitch 1",
	"/ocfoff": "cd ../ocfControl && ./ocfSwitch 0",
	"/onem2mon" : "cd ../onem2mControlAe && ./switch.sh 1",
	"/onem2moff" : "cd ../onem2mControlAe && ./switch.sh 0"
}

#This class will handles any incoming request from
#the browser 
class myHandler(BaseHTTPRequestHandler):
	
	#Handler for the GET requests
	def do_GET(self):
		if self.path=="/":
			self.path="/index.html"
		elif self.path in commands:
			self.send_response(200)
			Popen(commands[self.path], cwd=curdir, shell=True)
			return

		try:
			#Check the file extension required and
			#set the right mime type

			sendReply = False
			if self.path.endswith(".html"):
				mimetype='text/html'
				sendReply = True
			if self.path.endswith(".jpg"):
				mimetype='image/jpg'
				sendReply = True
			if self.path.endswith(".png"):
				mimetype='image/png'
				sendReply = True

			if sendReply == True:
				#Open the static file requested and send it
				f = open(curdir + sep + self.path) 
				self.send_response(200)
				self.send_header('Content-type',mimetype)
				self.end_headers()
				self.wfile.write(f.read())
				f.close()
			return


		except IOError:
			self.send_error(404,'File Not Found: %s' % self.path)

try:
	#Create a web server and define the handler to manage the
	#incoming request
	server = HTTPServer(('', PORT_NUMBER), myHandler)
	print 'Started httpserver on port ' , PORT_NUMBER
	
	#Wait forever for incoming htto requests
	server.serve_forever()

except KeyboardInterrupt:
	print '^C received, shutting down the web server'
	server.socket.close()

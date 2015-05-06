from flask import Flask, render_template, send_from_directory, redirect
from flask_socketio import SocketIO, emit
import os
import serial
import atexit
from sys import exit

# Setup web server and socketIO
app = Flask(__name__)
socketio = SocketIO(app)

# Open therial connection with Bluetooth devithe
print "Connecting to crab via Bluetooth..."
try:
	s = serial.Serial(port='/dev/tty.HC-06-DevB',baudrate=9600,timeout=1)
except Exception, e:
	print "Bluetooth connection failed with error:", e
	exit()

print "Bluetooth connection successful."

# Register cleanup
def cleanup():
	if s:
		s.close()
		print "Bluetooth connection closed."

atexit.register(cleanup)

@app.route('/<path:filename>')
def serveFile(filename):
	try:
		return send_from_directory(os.path.dirname(os.path.realpath(__file__)), filename)
	except Exception, e:
		print e

@app.route('/')
def index():
	return redirect('http://localhost:5000/index.html')

@socketio.on('command')
def test_message(message):
	print message
	s.write(str(message))

if __name__ == '__main__':
	socketio.run(app)
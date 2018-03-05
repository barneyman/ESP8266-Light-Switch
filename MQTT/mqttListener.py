import paho.mqtt.client as mqtt 
import time
import http.client
import json

def HandleCommand(object, command):
	data = json.load(open('config.json'))

	if "commands" in data:
		commands=data["commands"]
		if object in commands and command in commands[object]:
			for each in commands[object][command]:
				print (each['host'],each['url'])
				conn=http.client.HTTPConnection(each['host'])
				conn.request(url=each['url'], method="GET")

	


def on_message(mosq, obj, msg):
	print ("on_message:: this means  I got a message from broker for this topic")
	print(msg.topic + " " + str(msg.qos) + " " + str(msg.payload))

	topic=msg.topic.split("/")

	if len(topic)==3:
		HandleCommand(topic[2], msg.payload.decode('utf8'))

	return



def on_connect(client, userdata, flags, rc):
	print ("on_connect:: Connected with result code "+ str ( rc ) )
	print("rc: " + str(rc))
	print("" )
	client.subscribe ("barneyman/f/beachlights" ,1 )

def on_disconnect(mosq, obj, rc):
	print ("on_disconnect:: DisConnected with result code "+ str ( rc ) )
	print("rc: " + str(rc))
	print("" )
	print(client.reconnect());



def on_publish(mosq, obj, mid):
    print("mid: " + str(mid))
    print ("")

def on_subscribe(mosq, obj, mid, granted_qos):
    print("This means broker has acknowledged my subscribe request")
    print("Subscribed: " + str(mid) + " QoS " + str(granted_qos))

def on_log(mosq, obj, level, string):
    print(  string)


data = json.load(open('config.json'))
config=data["config"]


client_name=config["clientName"]
hostname=config["host"]
user=config["user"]
pwd=config["pwd"]
hostport=config["port"]

print(client_name,hostname,user,pwd,hostport)


client =mqtt.Client(client_name, clean_session=False)

client.username_pw_set(user, pwd)

client.on_message = on_message
client.on_connect = on_connect
client.on_publish = on_publish
client.on_subscribe = on_subscribe
client.on_disconnect = on_disconnect

# Uncomment to enable debug messages
client.on_log = on_log

# B64 encoded
client.tls_set("C:\\Users\\bjf\\Documents\\Arduino\\esp8266\\lightswitch\\adafruit_ca.crt")
client.connect(hostname, port=hostport)


client.loop_forever()

#client.subscribe ("/command" ,2 )
#client.subscribe ("barneyman/f/beachlights" ,2 )



#while True:
	#print (".")
	#client.publish  ( "barneyman/f/beachlights", "query" )
	#time.sleep(180)
#	client.loop()




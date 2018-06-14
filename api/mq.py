from Koj.settings import *
from api.models import *
import pika
import uuid
import json

class MQClient(object):
    def __init__(self):
        self.connection = pika.BlockingConnection(
            pika.ConnectionParameters(
                host=MQ_HOST,
                port=MQ_PORT,
                heartbeat_interval=0,
                socket_timeout=5,
                credentials=pika.PlainCredentials('guest', 'guest')))
        self.channel = self.connection.channel()
        self.callback_queue = self.channel.queue_declare('koj-resp').method.queue
        self.channel.basic_consume(self.on_response, no_ack=True,
                                   queue=self.callback_queue)
        self.response = None

    def on_response(self, ch, method, props, body):
        if self.corr_id == props.correlation_id:
            self.response = True
            data = json.loads(body.decode('utf8'))
            s = Submit.objects.get(id=self.sid)
            print([
                data.get('result'),
                data.get('time'),
                data.get('memory'),
                data.get('code_length'),
            ])
            s.result_info = data
            s.result = data.get('result')
            s.result_time = data.get('time')
            s.result_memory = data.get('memory')
            s.result_code_length = data.get('code_length')
            s.save()
            u = User.objects.get(id=self.uid)
            res = data.get('result')
            if res == 'AC':
                u.priority += 10
            elif res in ['SE', 'PE']:
                u.priority += 5
            elif res in ['CE', 'RE', 'MLE', 'OLE']:
                pass    # u.priority += 0
            else:   # WA/TLE
                u.priority += 2
            if u.priority > 100:
                u.priority = 100
            u.save()

    def call(self, submit, uid, sid, priority):
        self.response = None
        self.uid = uid
        self.sid = sid
        self.corr_id = str(uuid.uuid4())
        self.channel.basic_publish(exchange='',
                                   routing_key=MQ_QUEUE,
                                   body=json.dumps(submit),
                                   properties=pika.BasicProperties(
                                       reply_to = self.callback_queue,
                                       correlation_id = self.corr_id,
                                       priority=priority))
        while self.response is None:
            self.connection.process_data_events()
        self.channel.close()
        self.connection.close()

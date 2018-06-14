#!/usr/bin/env python3

import os
import sys
import time
import pika
import uuid
import json
import faker
import random
import requests
import threading
import numpy as np

# Urls
BASE = 'http://localhost:8000/api'
INFO = [
  BASE + '/languages/',
  BASE + '/me/',
]
RESC = [	# ?limit=?&offset=? || <pk>
  BASE + '/problems/',
  BASE + '/contests/',
  BASE + '/submits/',
  BASE + '/users/',
]
REG = BASE + '/users/'
AUTH = BASE + '/users/auth/'
SUBMIT = BASE + '/submit/'

# Codes
SRC = {
  'c': [
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x+y);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d %d", (x+y)/10, (x+y)%10);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x-y);\nreturn 0;\n}',
    'int main()\n{\nprintf("CE")\nreturn 0;\n}',
    'int main[65535]={0};',
    '#include<stdlib.h>\nint main()\n{\nint *p=1;\nfree(p);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nint x=0;\nwhile(1) x++;\nreturn 0;\n}',
    '#include<stdlib.h>\nint main()\n{\nint *p;\nwhile(1) p=malloc(1024);\nreturn 0;\n}',
    '#include<stdio.h>\nint main()\n{\nwhile(1) printf("OLE");\nreturn 0;\n}',
  ],
  'cpp': [
    '#include<iostream>\nusing namespace std;\n\nint main()\n{\nint x, y;\ncin>>x>>y;\ncout<<x+y;\nreturn 0;\n}',
    '#include<iostream>\nusing namespace std;\n\nint main()\n{\nint x, y;\ncin>>x>>y;\ncout<<" "<<x+y;\nreturn 0;\n}',
    '#include<iostream>\nint main()\n{\n  int x, y;\n  cin>>x>>y;\ncout<<x+y;\nreturn 0;\n}',
  ],
  'java': [
    'import java.io.*;\nimport java.util.*;\n\npublic class Main {\npublic static void main(String args[]) throws Exception {\nScanner scanner = new Scanner(System.in);\nint x = scanner.nextInt();\nint y = scanner.nextInt();\nSystem.out.println(x+y);\n}\n}',
    'public class Hello {public static void main(String args[]){}}',
    'public class Main {}',
  ],
  'python2': [
    'print sum([int(x) for x in raw_input().split()])',
    'print 233',
    'import exit\nsys.exit()'
  ],
  'python3': [
    'print(sum([int(x) for x in input().split()]))',
    'print("154")',
    'p()'
  ],
  'ruby': [
    'print(gets.split.map {|e| e.to_i}.reduce(:+))',
    'puts "233"',
    'system "kill 9"',
  ],
  'lua': [
    'x=io.read("*number")\ny=io.read("*number")\nio.write(x+y)',
    'waht',
    'print(os.clock())',
  ],
}
LANG = list(SRC)

class MQClient(object):
    def __init__(self):
        self.connection = pika.BlockingConnection(
            pika.ConnectionParameters(
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
            t = time.time() - self.time
            print('submit_roundtime: ' + str(t))
            log_fn = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                                  'judge', 'log', 'submit_roundtime.log')
            f = open(log_fn, 'a+')
            f.write(str(t))
            f.write('\n')
            f.flush()
            f.close()

    def call(self, submit):
        self.time = time.time()
        self.response = None
        self.corr_id = str(uuid.uuid4())
        self.channel.basic_publish(exchange='',
                                   routing_key='koj',
                                   body=json.dumps(submit),
                                   properties=pika.BasicProperties(
                                       reply_to = self.callback_queue,
                                       correlation_id = self.corr_id,
                                       priority=75))
        while self.response is None:
            self.connection.process_data_events()
        self.channel.close()
        self.connection.close()

class Visitor(threading.Thread):
  def __init__(self):
    super(Visitor, self).__init__()
    self.faker = faker.Faker()
    self.visited = []
    self.tid = self.getName()
    self.token = ''
    self.user = None
    self.headers = {
      # 'Authorization': 'Token ' + self.token,
      'X-Requested-With': 'XMLHttpRequest',
      'Content-Type': 'application/json'
    }


  def run(self):
    while True:
      act = random.random()
      if act <= 0.05:
        self.reg()    # 5%
      elif 0.05 < act <= 0.15:
        self.reauth() # 10%
      elif 0.15 < act <= 0.70:
        self.view()   # 55%
      else:
        pass
        # self.submit() # 30%
      self.cache_expire()
      time.sleep(np.random.poisson(25))

  def reg(self):
    self.user = {
      'username': self.faker.user_name(),
      'password': self.faker.password(),
      'email': self.faker.email(),
    }
    res = requests.post(REG, json=self.user)
    print('[%s] reg as %s | %s' % (self.tid, self.user['username'], res.content))
    self.auth()

  def reauth(self):
    self.user = {
      'username': 'test%d' % random.randint(0, 99),
      'password': 'test',
    }
    self.auth()

  def auth(self):
    res = requests.post(AUTH, json=self.user)
    self.token = json.loads(res.content.decode("utf-8"), encoding='utf-8').get('token')
    self.headers['Authorization'] = 'Token ' + self.token
    print('[%s] auth as %s | %s' % (self.tid, self.user['username'], res.content))

  def view(self):
    global count_req, count_cache

    res = None
    if random.random() <= 0.10: # info
      url = random.choice(INFO)
      if url not in self.visited:
        self.visited.append(url)
        res = requests.get(url, headers=self.headers)
    else:
      base = random.choice(RESC)
      if random.random() <= 0.4:  # detail view
        url = '%s%d/' % (base, random.randint(0, 150))
      else:                       # list view
        url = '%s?limit=20&offset=%d' % (base, random.randint(0, 8) * 20)
      if url not in self.visited:
        self.visited.append(url)
        res = requests.get(url, headers=self.headers)
    if res:
      count_req += 1
      print('[%s] visit %s | len=%s' % (self.tid, url, len(res.content)))
    else:
      count_cache += 1
      print('[%s] visit %s (cached)' % (self.tid, url))

  def submit(self):
    lang = random.choice(LANG)
    src = random.choice(SRC[lang])
    submit = {
      'uuid': str(uuid.uuid4()),
      'pid': 'test',  # str(p.sn),
      'src': src,
      'lang': lang,
      'time': 1000,
      'memory': 512 * 1024,
      'output': 32 * 1024,
    }
    print('[%s] submits with %s' % (self.tid, lang))
    def submit_thread(submit):
      start = time.clock()
      MQClient().call(submit)
      print('visitor submit roundtime' + str(time.clock() - start))
    threading.Thread(target=submit_thread, args=(submit,)).start()

  def cache_expire(self):
    upbound = len(self.visited) >> 2
    if upbound <= 0:
      return
    for i in range(random.randint(0, upbound)):
      self.visited.remove(random.choice(self.visited))

count_req = 0
count_cache = 0
log_fn = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
                      'judge', 'log', 'count_view_%d.log' % random.randint(1, 100))

log_fp = open(log_fn, 'a+')

def count_thread():
  global count_req, count_cache
  while True:
    log_fp.write("%d\t%d\n" % (count_req, count_cache))
    log_fp.flush()
    count_req = 0
    count_cache = 0
    time.sleep(5)

try:
  if len(sys.argv) >= 1:
    N = int(sys.argv[1])
    print('starts %d threads' % N)
  threading.Thread(target=count_thread).start()
  for i in range(N):
    Visitor().start()
except KeyboardInterrupt:
  log_fp.flush()
  log_fp.close()
  exit(0)

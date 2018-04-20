#!/usr/bin/env python3

import requests
import time
import random
import threading

N = 100
COUNT = 0
URL = [
  'http://localhost:8000/api/problems/',
  'http://localhost:8000/api/contests/',
  'http://localhost:8000/api/submits/',
]

class Visitor(threading.Thread):
  def run(self):
    while True:
      time.sleep(random.randint(3,10))
      url = random.choice(URL)
      resp = requests.get(url)
      global COUNT
      COUNT += 1
      print('[%d] %s' % (COUNT, self.getName()))

try:
  for i in range(N):
    v = Visitor()
    v.start()
except KeyboardInterrupt:
  exit(0)

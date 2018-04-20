#!/usr/bin/env python3

import sqlite3

db=sqlite3.connect("../db.sqlite3")
cur=db.cursor()
cur.execute("DELETE FROM api_submit;")
cur.close()
db.commit()
db.close()

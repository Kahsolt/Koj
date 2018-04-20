#!/usr/bin/env python3

import requests
import uuid
import time
import random

CODE = [
    # AC
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x+y);\nreturn 0;\n}',
    # PE
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d %d", (x+y)/10, (x+y)%10);\nreturn 0;\n}',
    # WA
    '#include<stdio.h>\nint main()\n{\nint x, y;\nscanf("%d%d", &x, &y);\nprintf("%d", x-y);\nreturn 0;\n}',
    # CE
    'int main()\n{\nprintf("CE")\nreturn 0;\n}',
    'int main[65535]={0};',
    # RE
    '#include<stdlib.h>\nint main()\n{\nint *p=1;\nfree(p);\nreturn 0;\n}',
    # TLE
    '#include<stdio.h>\nint main()\n{\nint x=0;\nwhile(1) x++;\nreturn 0;\n}',
    # MLE
    '#include<stdlib.h>\nint main()\n{\nint *p;\nwhile(1) p=malloc(1024);\nreturn 0;\n}',
    # OLE
    '#include<stdio.h>\nint main()\n{\nwhile(1) printf("OLE");\nreturn 0;\n}',
]
N = 0
while True:
    N += 1
    time.sleep(random.randint(1,3))
    submit = {
        'problem': 1000,
        'code': random.choice(CODE),
        'language': 'c',
    }
    headers = {
        'Authorization': 'Token 2f38de79-6dac-48d3-a1d4-637254bf918f',
        'X-Requested-With': 'XMLHttpRequest',
        'Content-Type': 'application/json',
    }
    url = 'http://localhost:8000/api/submit/'
    resp = requests.post(url, json=submit, headers=headers)
    print('[%d] %s' % (N, resp.content))

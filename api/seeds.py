import random
from .models import *
from faker import Faker
from django.db import transaction
from django.contrib.auth.models import User as AuthUser

@transaction.atomic
def seed():
    f = Faker()

    u = AuthUser.objects.create(username='kahsolt', password='', email='kahsolt@qq.com',
                                is_superuser=True, is_staff=True)
    u.set_password('kahsolt1379')
    u.save()

    User.objects.create(username='ks', password='1379', email='kahsolt@163.com')
    for i in range(100):
        User.objects.create(username="test%d"%i, password='test', email=f.email())

    for i in range(9000, 9025):
        Contest.objects.create(sn=i, title=f.sentence(), info=f.text(),
                               start_time=f.date_time(), end_time=f.date_time())
    for i in range(1000, 1150):
        c = None
        if random.random() <= 0.4:
            c = random.choice(Contest.objects.all())
        Problem.objects.create(sn=i, contest=c, title=f.sentence(), description=f.text(),
                               input_desc=f.text(), output_desc=f.text(),
                               sample_input=f.sentence(), sample_output=f.sentence(),
                               hint=f.sentence(), source=f.url())

    for i in range(350):
        u = random.choice(User.objects.all())
        p = random.choice(Problem.objects.all())
        res = random.choice(['AC', 'WA', 'CE', 'PE', 'RE', 'TLE', 'MLE', 'OLE', 'SE', 'Q'])
        Submit.objects.create(user=u, problem=p, timestamp=f.date_time(),
                              code=f.text(), language=f.city(),
                              result=res, result_code_length=random.randint(0, 1000),
                              result_info=f.text())
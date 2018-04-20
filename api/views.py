from rest_framework.decorators import list_route, detail_route
from rest_framework import viewsets, status
from .serializers import *
import threading
from .mq import MQClient

# Create your views here.
class UserViewSet(viewsets.ModelViewSet):
    queryset = User.objects.all()
    serializer_class = UserSerializer

    @list_route(methods=['POST'])
    def auth(self, request):
        data = request.data
        u = User.objects.filter(username=data.get('username'),
                                password=data.get('password')).first()
        if u:
            return JsonResponse({'token': u.token})
        else:
            return JsonResponse({}, status=status.HTTP_401_UNAUTHORIZED)

class ContestViewSet(viewsets.ReadOnlyModelViewSet):
    queryset = Contest.objects.all()
    serializer_class = ContestSerializer

class ProblemViewSet(viewsets.ReadOnlyModelViewSet):
    queryset = Problem.objects.all()
    serializer_class = ProblemSerializer

class SubmitViewSet(viewsets.ReadOnlyModelViewSet):
    queryset = Submit.objects.all()
    serializer_class = SubmitSerializer

# dynamic configured data, or aux functions
import json
from django.http.response import JsonResponse, HttpResponse
from django.shortcuts import redirect
from django.views.decorators.csrf import csrf_exempt

@csrf_exempt
def me(request):
    token = request.META.get("HTTP_AUTHORIZATION").split()[1]
    u = User.objects.filter(token=token).first()
    if not u:
        return JsonResponse({}, status=status.HTTP_401_UNAUTHORIZED)

    return JsonResponse({
        'username': u.username,
        'email': u.email,
        'priority': u.priority,
        'count_submit': u.count_submit,
        'count_ac': u.count_ac,
    })

@csrf_exempt
def submit(request):
    token = request.META.get("HTTP_AUTHORIZATION").split()[1]
    u = User.objects.filter(token=token).first()
    if not u:
        return JsonResponse({}, status=status.HTTP_401_UNAUTHORIZED)

    data = request.body.decode('utf-8')
    data = json.loads(data, encoding='utf8', parse_int=int, parse_float=float)
    p = Problem.objects.filter(sn=data.get('problem')).first()
    if not p:
        return JsonResponse({}, status=status.HTTP_404_NOT_FOUND)

    # Priority Strategy
    priority = u.priority
    if p.contest is not None:
        priority += 100
    else:
        u.priority -= 5
    if u.priority < 0:
        u.priority = 0
    u.save()
    s = Submit.objects.create(user=u, problem=p,
                              code=data.get('code'), language=data.get('language'))
    if not s:
        return JsonResponse({}, status=status.HTTP_422_UNPROCESSABLE_ENTITY)

    submit = {
        'uuid': str(uuid.uuid4()),
        'pid': 'test',  # str(p.sn),
        'src': s.code,
        'lang': s.language,
        'time': p.limit_time,
        'memory': p.limit_memory,
        'output': p.limit_output,
    }
    MQThread(submit, u.id, s.id, priority).start()
    return JsonResponse({"id": s.id})

class MQThread(threading.Thread):
    def __init__(self, submit, uid, sid, priority):
        super(MQThread, self).__init__()
        self.submit = submit
        self.uid = uid
        self.sid = sid
        self.priority = priority

    def run(self):
        MQClient().call(self.submit, self.uid,
                        self.sid, self.priority)


@csrf_exempt
def languages(request):
    langs = {
        'languages': [
            {'name': 'c',       'display_name': 'C'},
            {'name': 'cpp',     'display_name': 'C++'},
            {'name': 'java',    'display_name': 'Java'},
            {'name': 'python2', 'display_name': 'Python2'},
            {'name': 'python3', 'display_name': 'Python3'},
            {'name': 'ruby',    'display_name': 'Ruby'},
            # {'name': 'pascal',  'display_name': 'Pascal'},
            # {'name': 'perl',    'display_name': 'Perl'},
            # {'name': 'php',     'display_name': 'PHP'},
        ]
    }
    return JsonResponse(langs)

@csrf_exempt
def seed(request):
    from .seeds import seed
    seed()
    return redirect('/admin')
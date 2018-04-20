from django.db import models

# Create your models here.
class User(models.Model):
    username = models.CharField(max_length=64, unique=True, help_text='用户名')
    password = models.CharField(max_length=64, help_text='密码，暂时存平文本')
    token = models.CharField(max_length=36, null=True, blank=True, default=None, help_text='Ajax身份验证令牌')
    email = models.EmailField(null=True, blank=True, help_text='邮箱')
    # nickname = models.CharField(max_length=32, null=True, blank=True, help_text='昵称')
    # signature = models.CharField(max_length=128, null=True, blank=True, help_text='签名')
    priority = models.PositiveIntegerField(blank=True, default=75, help_text='积分优先级')
    count_submit = models.PositiveIntegerField(blank=True, default=0)
    count_ac = models.PositiveIntegerField(blank=True, default=0)

    def __str__(self):
        return "%s %d" %(self.username, self.priority)


class Contest(models.Model):
    sn = models.IntegerField(unique=True, help_text='序列号')
    title = models.CharField(max_length=64)
    info = models.TextField(null=True, blank=True, help_text='赛程简介')
    start_time = models.DateTimeField()
    end_time = models.DateTimeField()

    def __str__(self):
        return "[%d] %s" % (self.sn, self.title)

    class Meta:
        ordering = ('sn',)


class Problem(models.Model):
    contest = models.ForeignKey(Contest, null=True, blank=True, default=None, on_delete=models.SET_NULL)

    sn = models.IntegerField(unique=True, help_text='散题序列号/赛题相对序号')
    title = models.CharField(max_length=64)
    description = models.TextField()
    input_desc = models.TextField()
    output_desc = models.TextField()
    sample_input = models.TextField()
    sample_output = models.TextField()
    hint = models.CharField(max_length=256, null=True, blank=True)
    source = models.CharField(max_length=256, null=True, blank=True, help_text='题目来源')
    data = models.FileField(null=True, blank=True, help_text='测试数据，应当为zip压缩包，文件名为题目sn编号')
    limit_time = models.IntegerField(null=True, blank=True, default=1000, help_text='单位ms')
    limit_memory = models.IntegerField(null=True, blank=True, default=512*1024, help_text='单位KB')
    limit_output = models.IntegerField(null=True, blank=True, default=32*1024, help_text='单位KB')
    count_submit = models.PositiveIntegerField(blank=True, default=0, help_text='总提交数')
    count_ac = models.PositiveIntegerField(blank=True, default=0, help_text='总AC数')

    def __str__(self):
        return "[%d] %s" % (self.sn, self.title)

    class Meta:
        ordering = ('sn',)


class Submit(models.Model):
    user = models.ForeignKey(User, null=True, blank=True, on_delete=models.SET_NULL)
    problem = models.ForeignKey(Problem, on_delete=models.CASCADE)

    timestamp = models.DateTimeField(auto_now=True)
    code = models.TextField()
    language = models.CharField(max_length=16)
    RESULTS = (
        ('AC',  "Accepted"),
        ('CE',  "Compile Error"),
        ('RE',  "Runtime Error"),
        ('TLE', "Time Limit Exceed"),
        ('MLE', "Memory Limit Exceed"),
        ('OLE', "Output Limit Exceed"),
        ('PE',  "Presentation Error"),
        ('WA',  "Wrong Answer"),
        ('SE',  "System Error"),
        ('Q',   "Judging"),
    )
    result = models.CharField(max_length=4, choices=RESULTS, blank=True, default='Q')
    result_memory = models.PositiveIntegerField(null=True, blank=True)
    result_time = models.PositiveIntegerField(null=True, blank=True)
    result_code_length = models.PositiveIntegerField(null=True, blank=True)
    result_info = models.TextField(null=True, blank=True, help_text='评测机返回的JSON结果')

    def __str__(self):
       return "[%s] %s %s" % (self.problem.sn, self.user.username, self.result or '')

    class Meta:
        ordering = ('-timestamp',)


# Catch signal to auto-gen token for User
from django.db.models.signals import post_save
from django.dispatch import receiver
import uuid

@receiver(post_save, sender=User)
def create_token(sender, instance=None, created=False, **kwargs):
    if created and isinstance(instance, User):
        instance.token = str(uuid.uuid4())
        instance.save()
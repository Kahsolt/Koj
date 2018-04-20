from rest_framework import serializers
from .models import *

# Create your serializers here.
class UserSerializer(serializers.ModelSerializer):
    class Meta:
        model = User
        fields = ('__all__')

class ProblemSerializer(serializers.ModelSerializer):
    class Meta:
        model = Problem
        fields = ('__all__')

class ContestSerializer(serializers.ModelSerializer):
    problem_set = ProblemSerializer(many=True, read_only=True)

    class Meta:
        model = Contest
        fields = ('__all__')

class SubmitSerializer(serializers.ModelSerializer):
    class Meta:
        model = Submit
        fields = ('__all__')
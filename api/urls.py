from django.conf.urls import url, include
from rest_framework import routers
from .views import *

router = routers.DefaultRouter()
router.register(r'users', UserViewSet)
router.register(r'contests', ContestViewSet)
router.register(r'problems', ProblemViewSet)
router.register(r'submits', SubmitViewSet)

urlpatterns = [
    url(r'', include(router.urls)),

    url(r'^me/', me),
    url(r'^submit/', submit),
    url(r'^languages/', languages),
    url(r'^seed/', seed),
]
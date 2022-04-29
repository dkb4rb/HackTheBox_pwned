from django.urls import path

from . import views

urlpatterns = [
    path('<str:md5>/', views.view_file, name='upload')
]

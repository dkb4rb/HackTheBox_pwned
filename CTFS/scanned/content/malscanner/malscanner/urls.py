from django.contrib import admin
from django.urls import path, include, re_path

from . import views

urlpatterns = [
    path('scanner/', include("scanner.urls")),
    path('viewer/', include("viewer.urls")),
    path('admin/', admin.site.urls),
    re_path(r'^$', views.index, name='index'),
]

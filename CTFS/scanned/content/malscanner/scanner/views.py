from django.http import HttpResponse, HttpResponseRedirect
from django.shortcuts import render
from django.conf import settings
from django_file_md5 import calculate_file_md5

from .forms import UploadFileForm

import os


def index(request):
    return HttpResponse("Hello, world. You're at the polls index.")


def upload_file(request):
    if request.method == 'POST':
        form = UploadFileForm(request.POST, request.FILES)
        if form.is_valid():
            md5 = handle_file(request.FILES['file'])
            return HttpResponseRedirect(f'/viewer/{md5}')
        else:
            return HttpResponse("Invalid form")
    else:
        return render(request, 'upload.html', {'form': UploadFileForm()})


def handle_file(file):
    md5 = calculate_file_md5(file)
    path = f"{settings.FILE_PATH}/{md5}"
    with open(path, 'wb+') as f:
        for chunk in file.chunks():
            f.write(chunk)
    os.system(f"cd {settings.SBX_PATH}; ./sandbox {path} {md5}")
    os.remove(path)
    return md5

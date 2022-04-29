from django.shortcuts import render
from django.conf import settings
from django.http import Http404, HttpResponse
from .syscalls import LoggedSyscall, SyscallClass

import os.path
import struct


def view_file(request, md5: str):
    path = f"{settings.SBX_PATH}/jails/{md5}"
    if not os.path.exists(path):
        raise Http404("A sample with this hash has not been uploaded.")
    logfile = f"{path}/log"
    if not os.path.exists(logfile):
        return HttpResponse("There was an error logging this application")
    syscalls = [call.render() for call in parse_log(logfile)]
    ignore = list(filter(lambda call: call[0] == SyscallClass.Ignore, syscalls))
    low = list(filter(lambda call: call[0] == SyscallClass.Low, syscalls))
    med = list(filter(lambda call: call[0] == SyscallClass.Medium, syscalls))
    high = list(filter(lambda call: call[0] == SyscallClass.High, syscalls))
    render_vars = {"md5": md5, "ignore": ignore, "low": low, "med": med, "high": high}
    return render(request, 'view.html', render_vars)


def parse_log(path):
    syscalls = []
    with open(path, 'rb') as f:
        chunk = f.read(8 * 8)
        nums = struct.unpack("q" * 8, chunk)
        while len(chunk) == 8*8:
            nums = struct.unpack("q" * 8, chunk)
            call = LoggedSyscall(nums)
            syscalls.append(call)
            chunk = f.read(8 * 8)
    return syscalls

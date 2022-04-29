import numbers
import enum
from typing import List, Tuple


class SyscallClass(enum.Enum):
    Ignore = 0
    Low = 1
    Medium = 2
    High = 3

    def __gt__(self, other):
        if isinstance(other, numbers.Real):
            return self.value > other
        return self.value > other.value

    def __lt__(self, other):
        if isinstance(other, numbers.Real):
            return self.value < other
        return self.value < other.value


# Class, name, syscall number, arg count
syscalls = [
    [SyscallClass.Low, "read", 0, 3],
    [SyscallClass.Low, "write", 1, 3],
    [SyscallClass.Medium, "open", 2, 3],
    [SyscallClass.Low, "close", 3, 1],
    [SyscallClass.Medium, "stat", 4, 2],
    [SyscallClass.Medium, "fstat", 5, 2],
    [SyscallClass.Medium, "lstat", 6, 2],
    [SyscallClass.Medium, "access", 21, 2],
    [SyscallClass.Low, "alarm", 37, 1],
    [SyscallClass.High, "socket", 41, 3],
    [SyscallClass.High, "connect", 42, 3],
    [SyscallClass.High, "accept", 43, 3],
    [SyscallClass.High, "shutdown", 48, 2],
    [SyscallClass.High, "bind", 49, 3],
    [SyscallClass.High, "listen", 50, 2],
    [SyscallClass.Medium, "clone", 56, 5],
    [SyscallClass.Medium, "fork", 57, 0],
    [SyscallClass.Medium, "vfork", 58, 0],
    [SyscallClass.High, "execve", 59, 3],
    [SyscallClass.High, "kill", 62, 2],
    [SyscallClass.Medium, "uname", 63, 1],
    [SyscallClass.Medium, "getdents", 78, 3],
    [SyscallClass.Medium, "getcwd", 79, 2],
    [SyscallClass.Medium, "chdir", 80, 1],
    [SyscallClass.Medium, "fchdir", 81, 1],
    [SyscallClass.High, "rename", 82, 2],
    [SyscallClass.Low, "mkdir", 83, 2],
    [SyscallClass.High, "rmdir", 84, 1],
    [SyscallClass.High, "unlink", 87, 1],
    [SyscallClass.Medium, "chmod", 90, 2],
    [SyscallClass.Medium, "fchmod", 91, 2],
    [SyscallClass.High, "chown", 92, 3],
    [SyscallClass.High, "fchown", 93, 3],
    [SyscallClass.High, "ptrace", 101, 4],
]

template = """<div class="alert alert-{}" style="width: 80%" role="alert">
<p style="font-family: Courier New,Courier,Lucida Sans Typewriter,Lucida Typewriter,monospace;">{}({}) = {}</pre>
</div>
"""


class LoggedSyscall:
    sys_num: int
    rdi: int
    rsi: int
    rdx: int
    r10: int
    r8: int
    r9: int
    ret: int

    def __init__(self, values):
        self.sys_num, self.rdi, self.rsi, self.rdx, \
            self.r10, self.r8, self.r9, self.ret = values

    def get_args(self, count) -> List[int]:
        if count == 0:
            return []
        if count == 1:
            return [self.rdi]
        if count == 2:
            return [self.rdi, self.rsi]
        if count == 3:
            return [self.rdi, self.rsi, self.rdx]
        if count == 4:
            return [self.rdi, self.rsi, self.rdx, self.r10]
        if count == 5:
            return [self.rdi, self.rsi, self.rdx, self.r10, self.r8]
        if count == 6:
            return [self.rdi, self.rsi, self.rdx, self.r10, self.r8, self.r9]

    def render(self) -> Tuple[SyscallClass, str]:
        status = "light"
        for sys_entry in syscalls:
            if sys_entry[2] == self.sys_num:
                if sys_entry[0] == SyscallClass.Low:
                    status = "success"
                elif sys_entry[0] == SyscallClass.Medium:
                    status = "warning"
                elif sys_entry[0] == SyscallClass.High:
                    status = "danger"
                rendered = template.format(status, f"{sys_entry[1]}", ", ".join([
                    hex(x) for x in self.get_args(sys_entry[3])
                ]), hex(self.ret))
                return sys_entry[0], rendered
        rendered = template.format(status, f"sys_{self.sys_num}", "", hex(self.ret))
        return SyscallClass.Ignore, rendered

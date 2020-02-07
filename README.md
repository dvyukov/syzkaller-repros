# syzkaller bug reproducers

Collection of [syzkaller](https://github.com/google/syzkaller) bug reproducers for Linux kernel.\
A version of these reproducers also exists in [linux-arts](https://git.kernel.org/pub/scm/linux/kernel/git/shuah/linux-arts.git) repo.

Obtained from [syzbot](https://github.com/google/syzkaller/blob/master/docs/syzbot.md)
using [syz-reprolist](https://github.com/google/syzkaller/blob/64612bfd3c83d7246467cd9731f01a36b6331598/tools/syz-reprolist/reprolist.go).

To collect C reproducers from syz-manager workdir, run [./collect_reprogs.py /path/to/manager/workdir](collect_reprogs.py).

To build run [build.sh](build.sh).\
To run run [run.sh](run.sh).

[LTP](https://github.com/linux-test-project) is
[adding a wrapper](https://patchwork.ozlabs.org/patch/1204094/)
to run these tests, but it's not merged at the time of writing.

Testing should be done with KASAN, LOCKDEP, DEBUG_OBJECTS, DEBUG_VM, FAULT_INJECTION, lockup detectors
and [other debugging configs](https://github.com/google/syzkaller/blob/master/dashboard/config/bits-syzbot.config).

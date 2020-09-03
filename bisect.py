#!/usr/bin/env python3

import argparse
import glob
import filecmp
import logging
import os
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(
        description='Run syz-bisect for C/Syz reproducer')

    parser.add_argument('--reproducer', required=False,
                        help='Reproducer C source file')
    parser.add_argument('--reproducer_syz', required=False,
                        help='Reproducer syz file')
    parser.add_argument('--baseline_config', required=False,
                        help='Baseline configuration')
    parser.add_argument('--reproducer_config', required=True,
                        help='Reproducer configuration')
    parser.add_argument('--output', required=False, default="./out",
                        help='Output directory. default ./out')
    parser.add_argument('--kernel_repository', required=True,
                        help='Kernel git reprository')
    parser.add_argument('--kernel_branch', required=True,
                        help='Branch in kernel git reprository')
    parser.add_argument('--syzkaller_repository', required=False,
                        default="https://github.com/google/syzkaller.git",
                        help='Syzkaller git reprository')
    parser.add_argument('--syzkaller_branch', required=False,
                        default="HEAD",
                        help='Branch in  git reprository')
    parser.add_argument('--chroot', required=True,
                        help='Path to chroot environment')
    parser.add_argument('--sysctl', required=False,
                        default=os.path.dirname(__file__) + "/sysctl",
                        help='Path to sysctl file')
    parser.add_argument('--cmdline', required=False,
                        default=os.path.dirname(__file__) + "/cmdline",
                        help='Path to cmdline file')
    parser.add_argument('--bisect_bin', required=False, default="./bisect_bin",
                        help='Path to bisect_bin directory')
    parser.add_argument('--repro_opts', required=False,
                        default=os.path.dirname(__file__) + "/repro.opts",
                        help='Path to repro_opts file')
    parser.add_argument('--ccache', required=False, default="",
                        help='Path to ccache binary')
    args = parser.parse_args()

    if not args.reproducer and not args.reproducer:
        logging.error("Give either reproducer c source or syz file")
        exit(-1)

    outdir = os.path.abspath(args.output)
    os.makedirs(outdir, exist_ok=True)

    with open(os.path.dirname(__file__) + "/vm_syz-bisect.cfg") as cfg_template_file:
        cfg_template = cfg_template_file.readlines()

    vm_cfg_file = os.path.join(args.output, "vm.cfg")
    with open(vm_cfg_file, "w+") as cfg_file:
        kernel_source_dir = os.path.join(outdir, 'linux')
        syzkaller_source_dir = os.path.join(
            os.environ.copy()["HOME"],
            "go/src/github.com/google/syzkaller_bisect")
        workdir = os.path.join(outdir, 'workdir')
        image_dir = os.path.join(workdir, 'image')
        for line in cfg_template:
            line = line.replace("REPLACE_KERNEL_REPO", args.kernel_repository)
            line = line.replace("REPLACE_KERNEL_BRANCH", args.kernel_branch)
            line = line.replace("REPLACE_BISECT_BIN",
                                os.path.abspath(args.bisect_bin))
            line = line.replace("REPLACE_CCACHE_PATH",
                                args.ccache)
            line = line.replace("REPLACE_SYZKALLER_REPO",
                                args.syzkaller_repository)
            line = line.replace("REPLACE_SYSCTL", os.path.abspath(args.sysctl))
            line = line.replace("REPLACE_CMDLINE",
                                os.path.abspath(args.cmdline))
            line = line.replace("REPLACE_WORKDIR", os.path.join(outdir,
                                                                'workdir'))
            line = line.replace("REPLACE_KERNEL_OBJ", kernel_source_dir)
            line = line.replace("REPLACE_KERNEL_SOURCE", kernel_source_dir)
            line = line.replace("REPLACE_SYZKALLER", syzkaller_source_dir)
            line = line.replace("REPLACE_KERNEL",
                                os.path.join(kernel_source_dir,
                                             "arch/x86/boot/bzImage"))
            line = line.replace("REPLACE_USERSPACE",
                                os.path.abspath(args.chroot))
            line = line.replace("REPLACE_IMAGE",
                                os.path.join(image_dir, "image"))
            line = line.replace("REPLACE_KEY", os.path.join(image_dir, "key"))
            cfg_file.write(line)

    result = subprocess.run(["git", "ls-remote", args.kernel_repository,
                             args.kernel_branch], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    if result.returncode != 0:
        logging.error("git ls-remote failed for " + args.kernel_repository)
        exit(result.returncode)

    kernel_commit = result.stdout.decode("utf-8").strip().split("\n")[0].split()[0]

    with open(os.path.join(args.output, "kernel.commit"),
              "w+") as kernel_commit_file:
        kernel_commit_file.write(kernel_commit)

    result = subprocess.run(["git", "ls-remote", args.syzkaller_repository,
                             args.syzkaller_branch], stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE)
    if result.returncode != 0:
        logging.error("git ls-remote failed for " + args.syzkaller_repository)
        exit(result.returncode)

    syzkaller_commit = result.stdout.decode("utf-8").strip().split("\n")[0].split()[0]

    with open(os.path.join(args.output, "syzkaller.commit"),
              "w+") as syzkaller_commit_file:
        syzkaller_commit_file.write(syzkaller_commit)

    if args.baseline_config:
        shutil.copyfile(args.baseline_config,
                        os.path.join(outdir, "kernel.baseline_config"))
    shutil.copyfile(args.reproducer_config,
                    os.path.join(outdir, "kernel.config"))
    if args.reproducer:
        shutil.copyfile(args.reproducer, os.path.join(outdir, "repro.c"))
    if args.reproducer_syz:
        shutil.copyfile(args.reproducer_syz, os.path.join(outdir, "repro.syz"))
    shutil.copyfile(args.repro_opts, os.path.join(outdir, "repro.opts"))

    with open(os.path.join(outdir, "syz-bisect.log"), "w+") as syz_bisect_log:
        cmd = ["syz-bisect", "-config", os.path.basename(vm_cfg_file)]
        logging.info("Running: " + str(cmd))
        result = subprocess.run(cmd, stdout=syz_bisect_log,
                                stderr=syz_bisect_log, cwd=outdir)
        if result.returncode != 0:
            logging.error("syz-bisect failed")
            exit(result.returncode)


if __name__ == "__main__":
    logging.basicConfig(format='%(levelname)s: %(message)s',
                        level=logging.INFO)
    main()

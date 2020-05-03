#!/usr/bin/env python

import shlex
import sys
import json
import random
import subprocess
import os
import logging
import multiprocessing


def execute(cmd, **kwargs):
    if 'stdout' not in kwargs:
        kwargs['stdout'] = subprocess.PIPE
    if 'stderr' not in kwargs:
        kwargs['stderr'] = subprocess.PIPE
    if 'check' not in kwargs:
        kwargs['check'] = True
    try:
        logging.info('Running cmd {} with subprocess.run args {}'.format(cmd, kwargs))
        process = subprocess.run(shlex.split(cmd), **kwargs)
    except subprocess.CalledProcessError as exc:
        logged_stdout = None
        try:
            logged_stdout = exc.stdout
        except Exception:
            pass

        logged_stderr = None
        try:
            logged_stderr = exc.stderr
        except Exception:
            pass

        logging.exception('Command {} with STDOUT [{}], STDERR [{}] got exception:'.format(cmd, logged_stdout, logged_stderr))
        raise exc

    return process.stdout.decode('utf-8')


def dedup_cmd(args):
    (target_names, base_names) = args
    cmd = "./dedup " + " ".join(target_names + ["--"] + base_names)
    return cmd

def do_dedup(cmd):
    out = execute(cmd)
    return list(map(lambda x: float(x.strip()), filter(lambda x: len(x.strip()), out.split("\n"))))


def file_names(dir_name):
    return sorted(list(map(lambda x: os.path.join(dir_name, x), os.listdir(dir_name))))


def random_exp(pool, dir_name, select_from, select_to, num_targets):
    select_from = int(select_from)
    select_to = int(select_to)
    num_targets = int(num_targets)

    files = file_names(dir_name)
    random.shuffle(files)

    assert(num_targets + select_to <= len(files))

    base_names = []
    target_names = []
    for i in range(select_to - select_from + 1):
        current_bases = []
        for j in range(i + select_from):
            current_bases.append(files[j])
        base_names.append(current_bases)

        current_targets = []
        for j in range(num_targets):
            current_targets.append(files[-1 - j])
        target_names.append(current_targets)


    cmds = pool.map(dedup_cmd, list(zip(target_names, base_names)))
    res = pool.map(do_dedup, cmds)
    out = []
    for targets, base, cmd, results in zip(target_names, base_names, cmds, res):
        current = {"base": base, "results": {}, "cmd": cmd}
        for t, res in zip(targets, results):
            current["results"][t] = res
        out.append(current)

    print(json.dumps({"random_exp": out}, indent=True))


def main():
    args = sys.argv[1:]

    cores = int(args[0])
    args = args[1:]

    p = multiprocessing.Pool(cores)

    globals()[args[0] + "_exp"](p, *args[1:])


if __name__ == "__main__":
    main()

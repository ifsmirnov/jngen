#!/usr/bin/python

import re, os


HEADER_REGEX = re.compile('#include "(.*)"')


def is_header(line):
    res = HEADER_REGEX.match(line)
    if res:
        return res.groups()[0]


def direct_deps(filename):
    res = set()
    with open(filename) as fin:
        for line in fin.readlines():
            t = is_header(line)
            if t:
                res.add(t)
    return res


deps = {}


for filename in os.listdir('.'):
    if filename.endswith('.h') and filename != 'jngen.h':
        deps[filename] = direct_deps(filename)

order = []

while deps:
    for item in deps:
        if not deps[item]:
            order.append(item)
            del deps[item]
            for other in deps:
                deps[other].discard(item)
            break

with open("jngen.h", "w") as fout:
    for filename in order:
        with open(filename) as fin:
            for line in fin.readlines():
                if '#pragma once' not in line and not HEADER_REGEX.match(line):
                    fout.write(line)

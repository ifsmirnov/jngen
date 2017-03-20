#!/usr/bin/python

import re, os


HEADER_REGEX = re.compile('#include "(.*)"')


def extract_header(line):
    res = HEADER_REGEX.match(line)
    if res:
        return res.groups()[0]


def get_direct_deps(filename):
    res = set()
    with open(filename) as fin:
        for line in fin.readlines():
            t = extract_header(line)
            if t and not t.endswith("_inl.h"):
                res.add(t)
    return res


try:
    unused_files = set(map(str.strip, open(".unused_files").readlines()))
except IOError:
    unused_files = set()


unused_files.add("jngen.h")


deps = {}


for filename in os.listdir('.'):
    if filename.endswith('.h') and not filename.endswith("_inl.h") and\
            filename not in unused_files:
        deps[filename] = get_direct_deps(filename)

order = []

while deps:
    for item in sorted(deps):
        if not deps[item]:
            order.append(item)
            del deps[item]
            for other in deps:
                deps[other].discard(item)
            break


def write_file(filename, stream):
    with open(filename) as fin:
        for line in fin.readlines():
            include_or_not = HEADER_REGEX.match(line)
            if include_or_not:
                if include_or_not.groups()[0].endswith("_inl.h"):
                    write_file(include_or_not.groups()[0], stream)
            elif '#pragma once' not in line:
                stream.write(line)


with open("jngen.h", "w") as fout:
    for filename in order:
        write_file(filename, fout)

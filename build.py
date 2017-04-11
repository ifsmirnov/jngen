#!/usr/bin/python3


import re, os

HEADER_REGEX = re.compile('#include "(.*)"')

# This list may contain not all headers directly, but each jngen header
# must be among the dependencies of some file from here.
LIBRARY_HEADERS = [
    "array.h",
    "random.h",
    "common.h",
    "tree.h",
    "graph.h",
    "geometry.h",
    "math.h",
    "rnda.h",
    "rnds.h",
    "testcases.h",
    "options.h",
    "printers.h",
    "repr.h",
    "query_builder.h",
    "drawer/drawer.h",
]


def posix_path_to_native(posix_path):
    return os.path.join(*posix_path.split('/'))


def extract_header(line):
    res = HEADER_REGEX.match(line)
    if res:
        return res.groups()[0]


def extract_direct_deps(posix_filename):
    dir = os.path.dirname(posix_filename) # check explicitly on win

    res = set()
    with open(posix_path_to_native(posix_filename)) as fin:
        for line in fin.readlines():
            t = extract_header(line)
            if t and not t.endswith("_inl.h"):
                res.add(dir + '/' + t if dir else t)

    return res


deps = {}


def extract_deps(posix_filename):
    if posix_filename in deps:
        return deps[posix_filename]
    deps[posix_filename] = set((posix_filename,))

    for dep in extract_direct_deps(posix_filename):
        deps[posix_filename].update(extract_deps(dep))
    return deps[posix_filename]


def write_file(filename, stream):
    dir = os.path.dirname(filename) # check explicitly on win
    with open(posix_path_to_native(filename)) as fin:
        for line in fin.readlines():
            include_or_not = HEADER_REGEX.match(line)
            if include_or_not:
                if include_or_not.groups()[0].endswith("_inl.h"):
                    t = include_or_not.groups()[0]
                    write_file(dir + '/' + t if dir else t, stream)
            elif '#pragma once' not in line:
                stream.write(line)


headers = set()
for h in LIBRARY_HEADERS:
    headers.update(extract_deps(h))
headers = sorted(headers)

headers_in_order = []
while headers:
    for h in headers:
        if len(deps[h]) == 1:
            headers_in_order.append(h)
            for other in deps:
                deps[other].discard(h)
            del deps[h]
            headers.remove(h)
            break

with open("jngen.h", "w") as fout:
    for filename in headers_in_order:
        write_file(filename, fout)


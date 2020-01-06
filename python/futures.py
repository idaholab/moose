#!/usr/bin/env python
import os
import concurrent.futures
import threading
import time
from MooseDocs.tree import pages

"""
content = set()
for root, _, files in os.walk(os.path.join(os.getcwd(), 'MooseDocs', 'test', 'content')):
    for fname in files:
        if fname.endswith('.md'):
            content.add(pages.Source(fname, source=os.path.join(root, fname)))

def read(p):
    print(threading.current_thread().name, p.source, id(p))
    with open(p.source, 'r') as fid:
        return fid.read()


exe = concurrent.futures.ThreadPoolExecutor(max_workers=24)
start = time.time()

start = time.time()
data = dict()
for p, d in zip(content, exe.map(read, content)):
    data[p.uid] = d
print(time.time() - start)

for p in content:
    print(p.source, id(p))
"""

items = range(0, 100)
data = dict()

def compute(item):
    for j in range(0,10000):
        2**j

    data[item] = item*item



# Single
start = time.time()
for i in items:
    compute(i)
print(time.time() - start)


data.clear()


# Multiple
start = time.time()
exe = concurrent.futures.ProcessPoolExecutor(max_workers=10)
for j in exe.map(compute, items):
    pass
print(time.time() - start)
print(data)

#jobs = list()
#for i in items:
#    j = exe.submit(compute, i)
#    jobs.append(j)
#
#for j in jobs:
#    j.result()

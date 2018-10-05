#!/usr/bin/env python

import hit
import os, sys
from mooseutils import HitNode, hit_parse
from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse

if len(sys.argv) != 2:
  print "Usage: otter.py file.plot\n";
  sys.exit(1)

filename = sys.argv[1]

factory = Factory()
warehouse = Warehouse()
parser = Parser(factory, warehouse)

pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
pathname = os.path.abspath(pathname)
factory.loadPlugins([pathname], 'plugins', "IS_PLUGIN")

root_params = HitNode(hitnode=hit.parse('',''))
parser.parse(filename, root_params)

if parser.errors:
    for error in parser.errors:
        print error
    sys.exit(1)

active_objects = warehouse.getActiveObjects()
source_warehouse = {}

# initial setup
unordered_sources = []
for obj in active_objects:
    if obj._system == 'DataSource':
        unordered_sources.append(obj)
        source_warehouse[obj._name] = obj
    if obj._system == 'Output':
        obj.registerDataSourceWarehouse(source_warehouse)

# fill in actual objects for each dependency
for obj in unordered_sources:
    obj.applyDataSourceWarehouse(source_warehouse)

# dependency resolution between datasources
ordered_sources = []
while not not unordered_sources:
    candidates = []
    rejected = []
    # loop over all sources that still have not been processed
    for source in unordered_sources:
        # add sources to the candidates that only depend on objects already in ordered sources
        good = True
        for dep in source._source_dependencies:
            if source_warehouse[dep] not in ordered_sources:
                good = False
                break

        if good:
            candidates.append(source)
        else:
            rejected.append(source)

    if not candidates:
        print "Failed to resolve dependencies for " + ', '.join([obj._name for obj in unordered_sources])
        sys.exit(1)

    unordered_sources = rejected
    ordered_sources = ordered_sources + candidates

# execute datasources in dependency resolved order
for obj in ordered_sources:
    obj.execute()

# execute outputs
for obj in active_objects:
    if obj._system == 'Output':
        obj.execute()

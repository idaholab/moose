#!/usr/bin/env python3
import sys
from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader
from mooseutils.ReporterReader import ReporterReader
from math import isclose

def node_name(node):
    return 'Node {} with name "{}"'.format(node.id(), node.name())

def check_section(pgr, name):
    if not pgr.section(name):
        sys.exit(name + ' is not a section'.format(name))

def check_level(node, level):
    if node.level() != level:
        sys.exit(node_name(node) + ' should have a level of {}'.format(level))

def check_num_calls(node, num_calls):
    if node.numCalls() != num_calls:
        sys.exit(node_name(node) + ' should have {} calls'.format(num_calls))

def check_parent(node, parent_node):
    if node.parent() != parent_node:
        sys.exit(node_name(node) + ' has an incorrect parent')

def check_num_children(node, num_children):
    if len(node.children()) != num_children:
        sys.exit(node_name(node) + ' should have {} children'.format(num_children))

def check(file, pid, recover):
    pgr = PerfGraphReporterReader(file, part=pid)

    # The name of the root node
    root_name = 'MooseTestApp (main)'
    # The children that the root node should have (these may change over time)
    # The root node should have these two children (this may change over time)
    root_children_names = ['RankMap::construct', 'MooseApp::run']

    # Look for the root node we expect
    root_node = pgr.rootNode()
    check_section(pgr, root_name)
    if pgr.rootNode().name() != root_name:
        sys.exit(root_name + ' is not the root node')
    check_level(root_node, 0)
    check_num_calls(root_node, 2 if recover else 1)
    check_num_children(root_node, len(root_children_names))
    if not isclose(pgr.rootNode().percentTime(), 100):
        sys.exit('Root node percent time is not 100%')

    # Look for the children we expect
    for child in root_node.children():
        if child.name() not in root_children_names:
            sys.exit('Unexpected child "{}" in root node'.format(child.name()))
        check_num_calls(child, 2 if recover else 1)
        check_parent(child, root_node)

def main():
    recover = len(sys.argv) == 2 and sys.argv[1] == 'recover'

    file = 'perf_graph_reporter_' + ('recover_' if recover else '') + 'json.json'
    rr = ReporterReader(file)

    for pid in range(0, rr.numParts()):
        check(file, pid, recover)

if __name__ == '__main__':
    sys.exit(main())

#!/usr/bin/env python3
import sys
from mooseutils.PerfGraphReporterReader import PerfGraphReporterReader
from mooseutils.ReporterReader import ReporterReader

def check(file, pid, recover):
    # Load the test output; we do this first because we don't do diffs when
    # we have more than one processor but we can still make sure the output
    # is readable
    pgr = PerfGraphReporterReader(file, part=pid)

    if pid != 0:
        return

    # Load the gold file, from which will make sure that all of the sections
    # and nodes in the gold output also exist in the test output
    print('gold/' + file)
    gold_pgr = PerfGraphReporterReader('gold/' + file)

    errors = []
    def check_node(statement, gold_node, message):
        if not statement:
            errors.append('Node "' + '/'.join(gold_node.path()) + '": ' + message)
    def check_section(statement, gold_section, message):
        if not statement:
            errors.append('Section "' + gold_section.name() + '": ' + message)

    def act_node(gold_node):
        node = pgr.node(gold_node.path())
        if node is None:
            check_node(False, gold_node, 'Node is missing')
            return

        check_node(node.name() == gold_node.name(), gold_node, 'Name mismatch')
        check_node(node.level() == gold_node.level(), gold_node, 'Level mismatch')
        check_node(node.numCalls() == gold_node.numCalls(), gold_node, 'Num calls mismatch')
        check_node(len(node.children()) == len(gold_node.children()), gold_node, 'Number of children mismatch')

        for gold_child in gold_node.children():
            if pgr.node(gold_child.path()) is None:
                check_node(False, gold_node, 'Child "' + gold_child.name() + '" is missing')

        if gold_node.parent() is None:
            check_node(node.parent() == None, gold_node, 'Null parent mismatch')
        else:
            check_node(gold_node.parent().name() == node.parent().name(), gold_node, 'Parent mismatch')
    gold_pgr.recurse(act_node)

    for gold_section in gold_pgr.sections():
        section = pgr.section(gold_section.name())
        if section is None:
            check_section(False, gold_section, 'Section is missing')
        else:
            check_section(section.name() == gold_section.name(), gold_section, 'Name mismatch')
            check_section(section.level() == gold_section.level(), gold_section, 'Level mismatch')
            check_section(section.numCalls() == gold_section.numCalls(), gold_section, 'Num calls mismatch')
            check_section(len(section.nodes()) == len(gold_section.nodes()), gold_section, 'Number of nodes mismatch')

            for gold_node in gold_section.nodes():
                if section.node(gold_node.path()) is None:
                    check_section(False, gold_section, 'Node "' + gold_node.name() + '" is missing')

    for section in pgr.sections():
        if gold_pgr.section(section.name()) is None:
            check_section(False, gold_section, 'Extraneous section "' + section.name() + '"')

    if errors:
        print('Errors were found when diffing the graph from the gold file.')
        print('It is likely that you just need to regold "' + file + '".\n')
        print('\n'.join(errors))
        sys.exit()

def main():
    recover = len(sys.argv) == 2 and sys.argv[1] == 'recover'

    file = 'perf_graph_reporter_' + ('recover_' if recover else '') + 'json.json'

    rr = ReporterReader(file)

    for pid in range(0, rr.numParts()):
        check(file, pid, recover)

if __name__ == '__main__':
    sys.exit(main())

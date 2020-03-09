#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from optparse import OptionParser
import os, sys, re

"""*********************** The likely/unlikely optimizer ***********************
This script runs gcov on data from instrumented object files and parses the out-
put to determine candidates for optimization using the likely/unlikely macros.
Read include/base/Moose.h for documentation on these macros.

Run `LikelyOptimizer.py -h` for the options available for this script.

Before this script is run you must have compiled the object files you want to
optimize with `make METHOD=dbg coverage=true` and run a problem (or several
problems) so gcov has data about which branches are taken often.

Note: the percentage in parens is just the first percentage it finds, you must
inspect the .gcov file yourself to see what it means.  It is more complicated than
it looks because in a complicated if statement there may be several branches.
For example you may see something like this:
   617640:  128:    if ((neighbor->active() && (neighbor->level() == elem->level()) && (elem_id < neighbor_id)) || (neighbor->level() < elem->level()))
   call    0 returned 100%
   branch  1 taken 99% (fallthrough)
   branch  2 taken 1%
   call    3 returned 100%
   call    4 returned 100%
   branch  5 taken 99% (fallthrough)
   branch  6 taken 1%
   branch  7 taken 50% (fallthrough)
   branch  8 taken 50%
   etc....
so which branch is more likely??  It's not very clear.  The easiest way to tell
is to read the execution count of the if statement (617640), then find the next
line of code inside the if and see what happens there. In this example it is only
309000, meaning that the branch is only taken half the time and you should not
attempt to optimize with likely/unlikely.

Bugs and Limitations
There is no way to see just Moose code, you can get just the Moose .C files,
or Moose.C files and every .h file with -u -e
It doesn't look for else if statements, the assumptions is it will correctly
mark interesting if statements, and you will notice the else if yourself.
*******************************************************************************"""

# global options variable populated by main()
options = None

# returns a list of tuples of (path, filename)
def gather_files(dir, regex):
    prog = re.compile(regex)
    matches = []
    for (root, dirs, files) in os.walk(dir):
        for file in files:
            if prog.match(file):
                matches.append( (root, file) )

        if '.svn' in dirs:
            dirs.remove('.svn')

    return matches

# gcov has to be run file by file for some reason for the version on helios...
def run_gcov(files):
    branch_data = [] # tuple: (num_executions, file, line, code, percentage)
    for (dir, file) in files:
        command = 'cd %s; gcov -b -l %s' % (dir, file)
        print command
        os.popen(command).close() # we don't care about the output

        base = file.split('.')[0] + '.C'
        gcov = os.path.join(dir, file + '##' + base + '.gcov') # -l option means long filenames
        if not options.expand:
            branch_data += parse_gcov(gcov)

    return branch_data

def parse_gcov(fname):
    try:
        lines = open(fname).readlines()
    except:
        print 'Could not find file at ' + fname
        return []

    ifcheck = re.compile(r"^\s*\d+:\s*\d+:\s*if\s*\(") # matches " 12:  655: if ("
    branch = False      # boolean indicating that we just saw and if statement and are now looking for branch %s
    branch_tuple = None # (num_executions, file, line, code, percentage) of the current bracnh
    branch_data = []    # array of all branch data in this file
    orig_file = fname.split('#')[-1][:-5] # change blah.blah##foo.C.gcov to foo.C

    for line in lines:
        if branch:
            words = line.split()
            if words[0] != 'call' and words[0] != 'branch':
                branch = False
            if words[0] == 'branch' and words[-1] != 'executed': # it could be "branch # never executed"
                percentage = int( words[3][:-1] )
                if percentage > 50:
                    branch_tuple = branch_tuple + (percentage,)
                    branch_data.append(branch_tuple)
                    branch = False
                    #print branch_tuple
        elif ifcheck.match(line):
            words = line.split()
            execs = int( words[0][:-1] )
            num   = int( words[1][:-1] )
            code  = line.split(None, 2)[2].strip()
            branch_tuple = (execs, orig_file, num, code)
            branch = True             # inspect the next lines for branching data

    return branch_data

def print_results(results):
    print '\nFound %d if statements' % len(results)
    # sort by num_executions
    results = sorted(results, key = lambda a: -a[0])
    width = len(str(results[0][0])) + 1
    format = '%-' + str(width) + 'd (% 4d%%) %s %d: %s'

    print "Writing all branches ordered by execution count to '%s'" % options.output
    f = open(options.output, 'w')
    format = '%-' + str(width) + 'd (% 4d%%) %s %d: %s'
    formatted = [ format % (r[0], r[4], r[1], r[2], r[3]) for r in results ]
    f.write( '\n'.join(formatted) )
    f.close()

    # take the top chunk of results and sort them by file
    cutoff = min( len(results), options.min if options.min > 0 else int( options.perc / 100.0 * len(results) ) )
    results = results[:cutoff]
    results = sorted(results, key = lambda a: a[1])

    print 'execs ( perc) file line: if statement'
    for r in results:
        print format % (r[0], r[4], r[1], r[2], r[3])

def check_args(parser, options, args):
    if len(args) > 1:
        parser.error("Too many arguments")

    if options.expand or not options.gcov or options.filter:
        parser.error("This feature is not yet implemented.")

if __name__ == '__main__':
    parser = OptionParser(usage="usage: %prog [options] [start_dir]\nThis program will recursively search for all .gcno or .gcov files in start_dir.\nDefaults to the current directory.")
    parser.add_option("-c", "--show-completed", action="store_true", dest="filter", default=False, help="Show ifs that are already marked likely or unlikely [default=FALSE]")
    parser.add_option("-p", "--perc-branches", action="store", dest="perc", default=10, help="Only show the top percentage of branches [default=10%]")
    parser.add_option("-m", "--min-branches", action="store", dest="min", default=-1, help="Only show ifs that have been executed more than num times [default use percentage]")
    parser.add_option("-u", "--use-existing", action="store_false", dest="gcov", default=True, help="Don't run gcov, just parse all .gcov files already there")
    parser.add_option("-e", "--expanded-search", action="store_true", dest="expand", default=False, help="By default it will only look at .gcov files from .C or .gcno files, with this option look in *all* the generated .gcov files (ie included headers)")
    parser.add_option("-f", "--output-file", action="store", dest="output", default="likely_branches.out", metavar="FILE", help="outputs a list of all the branches ordered by execution counts")
    #parser.add_option("-t", "--num-threads", action="store", dest="threads", default=4, help="Number of threads to run gcov in parallel [default=4]")

    # parser args and exit if they don't have reasonable values
    (options, args) = parser.parse_args(sys.argv[1:])
    check_args(parser, options, args)

    dir = args[0] if len(args) == 1 else '.'

    results = None

    # Search for .gcno files and run gcov on all of them
    if options.gcov:
        print 'Searching %s for .gcno files' % dir
        files = gather_files(dir, r".+\.gcno$")
        print 'Running gcov on %d files' % len(files)
        if len(files) == 0:
            sys.exit(0)

        results = run_gcov(files)

    if options.expand or not options.gcov:
        # gather files
        results = None

    print_results(results)

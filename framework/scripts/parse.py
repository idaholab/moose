#!/usr/bin/python

##############################################################################
# Parser for Perf Log
#
# The following dictionary is a list of expressions to find in the
# output file.  The corresponding value will be encoded in the data
# file so that GNU plot can filter these lines for the right graphs
expr_to_find = {'Solve Only':-1, 'compute_residual':-6, 'solve()':-6, 'residual.close3()':-6}
##############################################################################

from subprocess import Popen, PIPE
from math import fabs
import re, sys

###########################################################
class AutoVivification(dict):
    """Implementation of perl's autovivification feature."""
    def __getitem__(self, item):
        try:
            return dict.__getitem__(self, item)
        except KeyError:
            value = self[item] = type(self)()
            return value
###########################################################

if len(sys.argv) < 2:
    print "Usage " + sys.argv[0] + " <directory_pattern>"
    sys.exit(0)

# This pattern finds numbers (ints and reals, but no exponents)
pattern = re.compile(r"\d+(?:\.\d+)?")
mpi_thread_pattern = re.compile(r"(\d+)_(\d+)x(\d+)x(\d+)")

# Look for all the expressions in the output and save them in a big data string
data = AutoVivification()
data_cores = AutoVivification()
data_strong = AutoVivification()
data_weak = AutoVivification()

for expr, field_num in expr_to_find.items():
    p = Popen('egrep "' + expr + '" ' +  sys.argv[1] + '/*/*.o*', shell=True, stdout=PIPE)
    output = p.communicate()[0]

    for line in output.split('\n'):
        if line == '': continue

        # Look for the generated folder for the mpi_procs and threads
        m = mpi_thread_pattern.search(line)

        problem_size       = m.group(1)
        chunks             = m.group(2)
        mpi_procs_per_node = m.group(3)
        threads            = m.group(4)

	# Calculated Values
	cores              = int(chunks) * int(mpi_procs_per_node) * int(threads)
        configuration      = mpi_procs_per_node + 'x' + threads

	expr_store = re.sub(r'\W', '', expr.replace(' ', '_'))

        values = pattern.findall(line)
	value = values[field_num]          # Only grab the one value we need counting from the back

        # Ahhh, Autovivication at it's best...
        data[expr_store][problem_size][chunks][mpi_procs_per_node][threads] = value
        data_cores[expr_store][problem_size][cores][configuration] = value
        data_strong[expr_store][problem_size][configuration][cores] = value
        data_weak[expr_store][configuration][problem_size][cores] = value

# Dump out all the raw data
data_out = open('data/all_data.dat', 'w')
data_out.write('problem_size chunks mpi_per_chunk threads perf_line timings\n')
for expr, level1 in data.items():
    for problem_size, level2 in sorted(level1.items(), key=lambda (k,v): int(k)):
        for chunks, level3 in sorted(level2.items(), key=lambda (k,v): int(k)):
            for mpi, level4 in sorted(level3.items(), key=lambda (k,v): int(k)):
                for threads, level5 in sorted(level4.items(), key=lambda (k,v): int(k)):
                    data_out.write(problem_size + ' ' + str(int(chunks) * int(mpi) * int(threads)) + ' ' + expr  + ' ' + level5 + '\n')
data_out.close()

for expr, level1 in data_cores.items():
    for problem_size, level2 in sorted(level1.items(), key=lambda (k,v): int(k)):

	configuration_set = {}
        for cores, level3 in sorted(level2.items(), key=lambda (k,v): int(k)):
            for configuration, level4 in level3.items():
                configuration_set[configuration] = True

        curr_file = open('data/' +  expr + '_' + str(problem_size) + '.dat', 'w')
        curr_file_norm = open('data/' + expr + '_' + str(problem_size) + '_norm.dat', 'w')
	curr_gnu_file = open('data/' + expr + '_' + str(problem_size) + '_log_plot.plt', 'w')
        curr_gnu_file.write("set datafile missing '-'\nset logscale\nset size square\nset style line 1 lt 3 lc rgb 'black' lw 3\n")
        curr_gnu_file.write("set title 'Log Log " + expr + ' ' + str(problem_size) + "'\n")

	curr_gnu_file.write("plot 10000*(x/1000)**-1 ti 'Ideal slope' ls 1")
        filename = expr + '_' + str(problem_size) + '.dat'
        for i in xrange(2, len(configuration_set)+2):
            curr_gnu_file.write(", \\\n '" + filename + "' using 1:" + `i` + ' ti col w lp')
            filename = ''

	curr_file.write(str('cores'))
	curr_file_norm.write(str('cores'))
	for configuration in sorted(configuration_set.keys(), key=lambda (k): int(k[0:k.index('x')])):
            curr_file.write(' ' + configuration)
            curr_file_norm.write(' ' + configuration)

        for cores, level3 in sorted(level2.items(), key=lambda (k,v): int(k)):
            curr_file.write('\n' + str(cores))
            curr_file_norm.write('\n' + str(cores))
            for configuration in sorted(configuration_set.keys(), key=lambda (k): int(k[0:k.index('x')])):
                if configuration in level3:
                    curr_file.write(' ' + level3[configuration])
                    curr_file_norm.write(' ' + str(float(level3[configuration])*float(cores)))
                else:
                    curr_file.write(' -')
                    curr_file_norm.write(' -')
        curr_file.close()
        curr_file_norm.close()

gnuplot_preamble = """
set yrange [0:1.5]
"""
#strong scaling data
gnuplot_plotline = "'<filename>' index <i> using 1:3 title 1 with linespoints"
for expr, problem_sizes in data_strong.items():
  for problem_size, configurations in sorted(problem_sizes.items(), key=lambda (k,v): int(k)):
    curr_file = open('data/' + expr + '_' + str(problem_size) + '_strong_scaling.dat', 'w')
    curr_gnu_file = open('data/' + expr + '_' + str(problem_size) + '_strong_scaling.plt', 'w')
    curr_gnu_file.write(gnuplot_preamble)
    curr_gnu_file.write('set title "Strong Scaling (' + expr + ' ' + str(problem_size) + ')"\n')

    count = 0
    for configuration, core_sets in sorted(configurations.items(), key=lambda (k,v): int(k[0:k.index('x')])):
      curr_file.write(str(configuration) + '\n')

      rel_speedup = '-'
      rel_eff = 1
      prev_cores = 1
      prev_time = 1
      for cores, time in sorted(core_sets.items(), key=lambda (k,v): int(k)):
        time = float(time)
        if prev_cores == 1:
          first_cores = cores
          first_time = time
        else:
          rel_speedup = first_time / time   #prev_time / time
          rel_eff = (first_cores * first_time) / (cores * time)  #(prev_cores * prev_time) / (cores * time)
        curr_file.write(str(cores) + ' ' + str(rel_speedup) + ' ' + str(rel_eff) + '\n')
        prev_cores = cores
        prev_time = time
      curr_file.write("\n\n")

      if not count:
        plotline = 'plot ' + gnuplot_plotline.replace('<filename>', expr + '_' + str(problem_size) + '_strong_scaling.dat')
      else:
        plotline = ', \\\n' + gnuplot_plotline.replace('<filename>', '')
      plotline = plotline.replace('<i>', `count`)
      count += 1
      curr_gnu_file.write(plotline)
      #curr_file.write('\n\n')

    curr_file.close()
    curr_gnu_file.close()

##weak scaling data
gnuplot_plotline = "'<filename>' index <i> using 1:3 title 1 with linespoints"
for expr, configurations in data_weak.items():
  for configuration, problem_sizes in configurations.items():
    curr_file = open('data/' + expr + '_' + str(configuration) + '_weak_scaling.dat', 'w')
    curr_gnu_file = open('data/' + expr + '_' + str(configuration) + '_weak_scaling.plt', 'w')
    curr_gnu_file.write('set title "Weak Scaling (' + expr + ' ' + str(configuration) + ')"\n')

    count = 0
    seen_it = set()
    for problem_size, core_sets in sorted(problem_sizes.items(), key=lambda (k,v): int(k)):
      for cores, time in sorted(core_sets.items(), key=lambda (k,v): int(k)):

        # For each problem size/core count see if we can find matches that keep the work load roughly the same
        #seen_it = set()
        new_set = True
        for problem_size2, core_sets2 in sorted(problem_sizes.items(), key=lambda (k,v): int(k)):
          for cores2, time2 in core_sets2.items():
            if (problem_size2, cores2) not in seen_it and \
                 int(problem_size2) > int(problem_size) and \
                 fabs(float(problem_size2)/float(problem_size) - float(cores2)/float(cores)) < 0.25: # How loose does this tol need to be?

              if new_set:
                new_set = False
                seen_it.add((problem_size, cores))
                curr_file.write('\n\n' + str(problem_size) + '_' + str(cores) + '\n' + str(cores) + ' ' + str(problem_size) + ' ' + str(time) + '\n')
                if count == 0:
                  plotline = 'plot ' + gnuplot_plotline.replace('<filename>', expr + '_' + str(configuration) + '_weak_scaling.dat')
                else:
                  plotline = ', \\\n' + gnuplot_plotline.replace('<filename>', '')
                plotline = plotline.replace('<i>', `count`)
                count += 1
                curr_gnu_file.write(plotline)

              seen_it.add((problem_size2, cores2))
              curr_file.write(str(cores2) + ' ' + str(problem_size2) + ' ' + str(time2) + '\n')

    curr_file.close()
    curr_gnu_file.close()

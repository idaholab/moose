import subprocess
import sys
import argparse

def str2bool(v):
    if isinstance(v, bool):
       return v
    if v.lower() in ('yes', 'true', 't', 'y', '1'):
        return True
    elif v.lower() in ('no', 'false', 'f', 'n', '0'):
        return False
    else:
        raise argparse.ArgumentTypeError('Boolean value expected.')

parser = argparse.ArgumentParser(description='Bulk run_tests')
parser.add_argument('-j', nargs='?', metavar='int', action='store', type=int, dest='jobs', default=1, help='value for -j in run_tests')
parser.add_argument('--pmin', nargs='?', metavar='int', action='store', type=int, dest='pmin', default=1, help='min value for -p in run_tests')
parser.add_argument('--pmax', nargs='?', metavar='int', action='store', type=int, dest='pmax', default=5, help='max value for -p in run_tests')
parser.add_argument("--opt", type=str2bool, nargs='?', const=True, default=False, dest='opt', help="whether or not to run opt mode tests")
parser.add_argument("--devel", type=str2bool, nargs='?', const=True, default=True, dest='devel', help="whether or not to run devel mode tests")
parser.add_argument("--dbg", type=str2bool, nargs='?', const=True, default=False, dest='dbg', help="whether or not to run dbg mode tests")
parser.add_argument("--distributed-mesh", type=str2bool, nargs='?', const=True, default=True, dest='distributed_mesh', help="whether or not to run distributed mesh tests")
parser.add_argument("--replicated-mesh", type=str2bool, nargs='?', const=True, default=True, dest='replicated_mesh', help="whether or not to run replicated mesh tests")
parser.add_argument('--n-threads', nargs='?', metavar='int', action='store', type=int, dest='n_threads', default=2, help='whether or not to run threaded tests (1 or 0 implies not to)')
parser.add_argument('--re', action='store', type=str, dest='reg_exp', help='Run tests that match --re=regular_expression')
parser.add_argument("--lcov", type=str2bool, nargs='?', const=True, default=False, dest='lcov', help="whether or not to use for building coverage")
parser.add_argument('--dbg-max-p', nargs='?', metavar='int', action='store', type=int, dest='dbg_max_p', default=2, help='maximum number of -p to run for dbg')

args = parser.parse_args()

options_list = []
for method in ['dbg', 'devel', 'opt']:
    if (getattr(args, method)):
        base_options = ['--{}'.format(method)]
        if args.reg_exp != None:
            base_options.append('--re={}'.format(args.reg_exp))
        if args.replicated_mesh:
            options_list.append(base_options)
        if args.distributed_mesh:
            append_options_copy = base_options.copy()
            append_options_copy.append('--distributed-mesh')
            options_list.append(append_options_copy)
        if args.n_threads > 1:
            for i in range(2, args.n_threads + 1):
                append_options_copy = base_options.copy()
                append_options_copy.append('--n-threads={}'.format(i))
                options_list.append(append_options_copy)
print("Running with options:")
for options in options_list:
    print(*options, sep=' ')
print()

procs_list = list(range(args.pmin, args.pmax + 1))

def run_coverage(options):
    command = ['framework/scripts/build_coverage.py']
    for option in options:
        command.append(option)
    print(*command)
    process = subprocess.run(command, cwd='../..')
    if process.returncode is not 0:
        sys.exit('Exiting due to nonzero return code')
    print()

trace_i = 0
for options in options_list:
    for procs in list(range(args.pmin, args.pmax + 1)):
        if "".join(options).count('dbg') and procs > args.dbg_max_p:
            continue

        if args.lcov:
            print("Initializing coverage:")
            run_coverage(['--mode', 'initialize', '--application', 'modules/ray_tracing'])

        command = ['./run_tests', '-j {}'.format(args.jobs), '-p {}'.format(procs), '-t']
        for option in options:
            command.append(option)
        print(*command)
        process = subprocess.run(command)

        if process.returncode is not 0:
            sys.exit('Exiting due to nonzero return code')

        if args.lcov:
            print("\nGenerating for coverage:")
            run_coverage(['--mode', 'generate',
                          '--outfile', 'ray_tracing_trace_{}'.format(trace_i),
                          '--application', 'modules/ray_tracing'])
            trace_i += 1

if args.lcov:
    trace_string = ''
    for i in range(0, trace_i):
        trace_string += ' ray_tracing_trace_{}'.format(i)

    print("Generating combined coverage:")
    combine_command = ['--mode', 'combine',
                       '--generate-html',
                       '--html-location', 'ray_tracing_coverage',
                       '--outfile', 'ray_tracing_trace_combined',
                       '--title', 'ray_tracing',
                       '--add-tracefile']
    for i in range(0, trace_i):
        combine_command.append('ray_tracing_trace_{}'.format(i))
    run_coverage(combine_command)

    print("Verifying coverage")
    run_coverage(['--verify-coverage',
                  '--application', 'modules/ray_tracing',
                  '--outfile', 'ray_tracing_trace_combined'])

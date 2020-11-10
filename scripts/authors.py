#!/usr/bin/env python3
import os
import collections
import argparse
import multiprocessing
import mooseutils

# List of available languages and an associated function for testing if a filename is that language
LANGUAGES = collections.OrderedDict()
LANGUAGES['C++'] = lambda f: f.endswith(('.C', '.h'))
LANGUAGES['Python'] = lambda f: f.endswith('.py')
LANGUAGES['Input'] = lambda f: f.endswith(('.i', '.hit'))
LANGUAGES['Markdown'] = lambda f: f.endswith('.md')
LANGUAGES['Make'] = lambda f: f.endswith(('Makefile', '.mk'))
LANGUAGES['YAML'] = lambda f: f.endswith('.yml')

def get_options():
    """Return the command-line options"""
    parser = argparse.ArgumentParser(description='Tool for listing author line counts.',
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('locations', nargs='*', type=str, default=[mooseutils.git_root_dir()],
                        help='The repository directory to consider.')
    parser.add_argument('-j', '--num-threads', type=int, default=os.cpu_count(),
                        help="The number of threads to use for computing the counts.")
    parser.add_argument('--exclude', nargs=1, type=str, default='contrib',
                        help="Exclude pattern passed to git ls-files call.")
    parser.add_argument('-l', '--languages', nargs='+', type=str, choices=list(LANGUAGES.keys()),
                        default=list(LANGUAGES.keys()),
                        help="Limit the analysis the the listed languages.")

    return parser.parse_args()

def target(filename):
    """Helper for counting the lines, by author of the given filename"""
    return mooseutils.git_lines(filename)

def update_count(c, lang, counts):
    """
    Add the counts from to the total count

    Input:
       c[dict]: Local counts with authors as keys, returned from 'target' function
       lang[str]: The language key that the 'c' count dict is associated
       counts[dict of dict]: The global count by author, then language
    """
    for key, value in c.items():
        counts[key][lang] += value

def report(counts):
    """
    Prints the global count in a table on the screen
    """
    titles = list(list(counts.values())[0].keys()) + ['Total']
    row_format = '{:>25}'
    row_format += "{:>10}" * (len(titles))
    n = 25 + 10 * len(titles)
    totals = {k:0 for k in titles}
    print('-'*n)
    print(row_format.format("Name", *titles))
    print('-'*n)

    for author, row in reversed(sorted(counts.items(), key=lambda item:sum(item[1].values()))):
        row['Total'] = sum(row.values())
        values = ['{:,}'.format(row[key]) for key in titles]
        for key in titles:
            totals[key] += row[key]
        print(row_format.format(author, *values))
    print('-'*n)
    print(row_format.format('TOTAL', *['{:,}'.format(totals[key]) for key in titles]))

if __name__ == '__main__':
    args = get_options()

    # Populate desired langauges
    lang = collections.OrderedDict()
    for key in args.languages:
        lang[key] = LANGUAGES[key]

    # List all files in the repository
    all_files = set()
    for location in args.locations:
        all_files.update(mooseutils.git_ls_files(os.path.abspath(args.locations[0]), exclude=args.exclude))

    # Group filenames by extension
    groups = collections.defaultdict(list)
    for filename in all_files:
        for key, func in lang.items():
            if func(filename):
                groups[key].append(filename)

    # Report author counts by file type
    counts = collections.defaultdict(lambda: {g:0 for g in lang.keys()})
    for group, files in groups.items():
        print('Counting {} lines...'.format(group), end='')
        with multiprocessing.Pool(processes=args.num_threads) as pool:
            for c in pool.imap_unordered(target, files):
                update_count(c, group, counts)
        print('done')
    report(counts)

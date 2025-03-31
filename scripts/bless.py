#!/usr/bin/env python3
''' A tool which searches recursively and removes duplicate loader paths '''
import subprocess
import sys
import os

def run_command(command):
    """Run a shell command and return the output."""
    try:
        result = subprocess.run(command, check=True, text=True, capture_output=True)
        return result.stdout
    except subprocess.CalledProcessError as e:
        print(f"Error running command: {' '.join(command)}")
        print(e.stderr)
        sys.exit(1)

def get_all_libraries(binary_file, paths=None)->set:
    """ Retrieve all libraries involved """
    if paths is None:
        paths = set([])
    output = run_command(['otool', '-l', binary_file])
    name_entries = set([])
    path_entries = set([])
    for line in output.splitlines():
        if ' name ' in line:
            name = line.split(' ')[-3]
            name_entries.add(name.replace('@rpath/', ''))
        if ' path /' in line:
            path = line.split(' ')[-3]
            path_entries.add(path)
    for lib in name_entries:
        if os.path.exists(lib):
            paths.add(lib)
        for l_path in path_entries:
            _tmp_path = os.path.join(l_path, lib)
            if os.path.exists(_tmp_path) and _tmp_path not in paths:
                paths.add(_tmp_path)
                paths.update(get_all_libraries(_tmp_path, paths))
    return paths

def find_and_fix_duplicates(binary_file):
    """Remove duplicate LC_RPATH entries from a binary using install_name_tool."""
    # Discover duplicate
    output = run_command(['otool', '-l', binary_file])
    paths = []
    bad_paths = set([])
    for line in output.splitlines():
        if ' path ' in line:
            path = line.split(' ')[-3]
            paths.append(path)

    # get duplicates from a list
    seen = set()
    bad_paths = [x for x in paths if x in seen or seen.add(x)]

    # Fix any libraries with duplicates
    for rpath in bad_paths:
        command = ['install_name_tool', '-delete_rpath', rpath, binary_file]
        run_command(command)
        print(f"Removed duplicate RPATH entries from: {binary_file}")

def process_input(input_stream):
    """ yield stdin lines as a generator object """
    yield from input_stream

def is_file(a_file)->bool:
    """ check if supplied file is in fact a file """
    if not os.path.exists(a_file):
        print(f'Error: The file {a_file} does not exist.')
        return False
    elif not os.path.isfile(a_file):
        print(f'{a_file} not a file')
        return False
    return True

def main():
    """ Main function to process the binary file """
    if not os.isatty(0):
        for b_file in process_input(sys.stdin):
            _tmp_file = b_file.strip()
            if is_file(_tmp_file):
                paths = get_all_libraries(_tmp_file)
                for path in paths:
                    find_and_fix_duplicates(path)
        sys.exit()
    elif len(sys.argv) != 2:
        print("Usage: python fix_rpaths.py <path_to_binary>")
        sys.exit(1)

    b_file = sys.argv[1]
    if is_file(b_file):
        paths = get_all_libraries(b_file)
        for path in paths:
            find_and_fix_duplicates(path)

if __name__ == "__main__":
    main()

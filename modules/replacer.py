# python 3
from subprocess import Popen, PIPE
import re

pattern= 'to_multiapp'
new_name = 'to_multi_app'
eq = ' ='
eq2 = ' ='

# Find all files with the offending pattern
command = "grep -r " + pattern + "| awk '{print substr( $1, 0, length($1) - 1) }' | uniq | grep -v inar | grep '\.i'"
with Popen(command, stdout=PIPE, stderr=None, shell=True) as process:
    output = process.communicate()[0].decode("utf-8")

# Open files one by one and change it
file_list = output.split('\n')[:-1]
for file_name in file_list:

    file = open(file_name, "rt+")
    file_lines = file.readlines()
    # print(file)

    # for each line in the input file
    for i, line in enumerate(file_lines[:-1]):

        # Find pattern
        if ('direction'+eq) in line and ('TO_' in line or 'to_' in line):

            # Look in previous lines for the other pattern
            ind = i
            found = False
            line_search = ''
            while '[' not in line_search and not found:
                line_search = file_lines[ind - 1]
                ind -= 1
                print(line_search)

                if 'multi_app' in line_search and '[' not in line_search:
                    found = True
                    next_line = line_search
            # Look in next lines
            if not found:
                ind = i
            line_search = ''
            while '[' not in line_search and not found:
                line_search = file_lines[ind + 1]
                ind += 1
                print(line_search)

                if 'multi_app' in line_search and '[' not in line_search:
                    found = True
                    next_line = line_search

            # print(line, next_line)
            # This is where each part of the pattern is
            ind_dir = i
            ind_app = ind

            # Remove direction
            file_lines[ind_dir] = ''

            # Change app indication
            multiapp_name = next_line.split(eq)[-1]
            leading_spaces = next_line[:(len(next_line) - len(next_line.lstrip()))]
            file_lines[ind_app] = leading_spaces + new_name + ' =' + multiapp_name

            # print(file_lines[ind_dir], file_lines[ind_app])


    # return pointer to top of file so we can re-write the content with replaced string
    file.seek(0)
    # clear file content
    file.truncate()
    # re-write the content with the updated content
    file.write(''.join(file_lines))
    # close file
    file.close()

    # break

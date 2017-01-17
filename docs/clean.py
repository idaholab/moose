import os
import subprocess

stub = 'Remove this when content is added. -->'


for base, _, files in os.walk('content', topdown=False):
    for fname in files:
        filename = os.path.join(base, fname)
        if fname.endswith('.md'):
            with open(filename, 'r') as fid:
                content = fid.read()

            if stub in content:
                os.remove(filename)
                #subprocess.call(['git', 'rm', filename])

        if fname == 'pages.yml':
            os.remove(filename)
            #subprocess.call(['git', 'rm', filename])

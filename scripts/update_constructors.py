#!/usr/bin/env python
import os, sys, re

# Helper functions for reading/writing files
def read(filename):
    fid = open(filename, 'r')
    content = fid.read()
    fid.close()
    return content
def write(filename, content):
    fid = open(filename, 'w')
    fid.write(content)
    fid.close()

# Substitution function for source file
def subSource(match):
    find = match.group(0)
    repl = match.group(1) + '::' + match.group(1) + '(const InputParameters & ' + match.group(2) + ')'
    repl += match.group(3) + match.group(4) + '(' + match.group(2) + ')'
    return match.group(0).replace(find, repl)

# Substitution function for source file for Actions and App
def subSourceNonConst(match):
    find = match.group(0)
    repl = match.group(1) + '::' + match.group(1) + '(InputParameters ' + match.group(2) + ')'
    repl += match.group(3) + match.group(4) + '(' + match.group(2) + ')'
    return match.group(0).replace(find, repl)


# Substitution function for header file
def subHeader(match):

    find = match.group(0)
    repl = match.group(1) + '(const InputParameters & ' + match.group(2) + ')'
    return match.group(0).replace(find, repl)

# Substitution function for header file for Actions and App
def subHeaderNonConst(match):

    find = match.group(0)
    repl = match.group(1) + '(InputParameters ' + match.group(2) + ')'
    return match.group(0).replace(find, repl)

# Updates the source code (.C) files
def updateSource(obj, filename):

    content = read(filename)
    obj = os.path.splitext(obj)[0]

    regex = r'(' + obj + ')\s*::\s*' + obj + '\s*\('
    regex += '\s*const\s*std\s*::\s*string\s*&\s*\w+'
    regex += '[\s\n]*,[\s\n]*InputParameters\s*(\w+)\s*'
    regex += '\s*\)([\n\s]*:[\s\n]*)'
    regex += '([\w\<\>\s]+)\(\s*\w+\s*,\s*\w+\s*\)'

    if (obj.endswith('App') and not obj.endswith('MultiApp')) or ('actions' in filename):
        content = re.sub(regex, subSourceNonConst, content, flags=re.MULTILINE|re.DOTALL)
    else:
        content = re.sub(regex, subSource, content, flags=re.MULTILINE|re.DOTALL)

    write(filename, content)

# Updates the header code (.h) files
def updateHeader(obj, filename):

    updateSource(obj, filename) # catches constructors defined in header

    content = read(filename)
    obj = os.path.splitext(obj)[0]

    regex = r'(' + obj + ')\s*\(\s*'
    regex += 'const\s*std\s*::\s*string\s*&\s*\w+'
    regex += '[\n\s]*,[\s\n]*InputParameters\s*(\w+)\s*\)'

    if (obj.endswith('App') and not obj.endswith('MultiApp')) or ('actions' in filename):
        content = re.sub(regex, subHeaderNonConst, content, flags=re.MULTILINE|re.DOTALL)
    else:
        content = re.sub(regex, subHeader, content, flags=re.MULTILINE|re.DOTALL)

    write(filename, content)

# Main program, loop over all files in the current directory and update the constructors
if __name__ == "__main__":
    for root, dirs, files in os.walk(os.getcwd()):
        for f in files:
            fname = os.path.join(root,f)
            if fname.endswith('.C'):
                print fname
                updateSource(f, fname)
            if fname.endswith('.h'):
                print fname
                updateHeader(f, fname)

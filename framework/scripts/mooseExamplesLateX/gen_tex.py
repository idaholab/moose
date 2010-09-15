#!/usr/bin/env python

import os, re
from subprocess import Popen

moose_examples_dir = os.environ['HOME'] + "/projects/gnu/trunk/moose_examples"

preamble_file = 'preamble.tex'
line_template = '\\lstinputlisting[style=<STYLE>, caption=<CAPTION>]{<PATH>}\n\clearpage\n\n'

def genPreamble(out_file):
  in_file = open(preamble_file, 'r')
  out_file.write(in_file.read())

def genFileList(out_file):
  for name in os.listdir(moose_examples_dir):
    if os.path.isdir(moose_examples_dir + '/' + name):
      readOrTraverseDirectory(out_file, moose_examples_dir, name)
       

def readOrTraverseDirectory(out_file, dirpath, dir):
  # First make sure that this is an example directory (i.e. of the form exXX where XX are digits)
  m = re.search(r'ex0?(\d+)', dir)
  if not m:
    return

  # get the example number 
  example_number = m.group(1)

  # see if there is a file list
  curr_path = dirpath + '/' + dir
  if os.path.isfile(curr_path + '/training_list.txt'):
    f = open(curr_path + '/training_list.txt')
    for line in f.readlines():
      line = line.strip()
      if os.path.isfile(curr_path + '/' + line):
        writeTex(out_file, curr_path + '/' + line, example_number)
      elif line <> '': # ignore blank lines
        print 'Warning: File ' + curr_path + '/' + line + ' does not exist\n'
  
  # file list doesn't exist so recurse and pick up the common files
  else: 
    for dirpath, dirnames, filenames in os.walk(curr_path):
      for file in filenames:
        suffix = os.path.splitext(file)
        if suffix[-1] == '.C' or suffix[-1] == '.h' or suffix[-1] == '.i':
          writeTex(out_file, dirpath + '/' + file, example_number)


def writeTex(out_file, file_name, example_number):
  file_only = file_name.split("/")[-1].replace('_', r'\_')
  suffix = os.path.splitext(file_name) 

  #First substitute in the file name
  text = line_template.replace('<PATH>', file_name)
          
  #Now substitute in the caption
  text = text.replace('<CAPTION>', 'Example ' + example_number + ': ' + file_only)

  if (suffix[-1] == '.h' or suffix[-1] == '.C'):
    style = 'C++'
  elif (suffix[-1] == '.i'):
    style = 'ini'

  #Now substitute in the sytle
  text = text.replace('<STYLE>', style)

  out_file.write(text)


def genPostscript(out_file):
  out_file.write('\n\\end{document}')

if __name__ == '__main__':
  tex_file = open('examples.tex', 'w')

  genPreamble(tex_file)
  genFileList(tex_file)
  genPostscript(tex_file)

  p = Popen('pdflatex examples.tex', shell=True)

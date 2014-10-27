#!/usr/bin/python
import fileinput
import sys
import os
import re
import math
import numpy as np
import subprocess

from optparse import OptionParser


nlsRE = re.compile("^Nonlinear System:$")
varlistFullRE = re.compile("^\s*Variables:\s*\{([^}]+)\}")
varlistStartRE = re.compile("^\s*Variables:\s*\{([^}]+)")
varlistEndRE = re.compile("^\s*([^}]+)\}")
varlistContRE = re.compile("^\s*([^}]+)")
varRE = re.compile("\"([^\"]+)\"")
dofRE  = re.compile("\s*Num DOFs:\s*([\d]+)")
ldofRE = re.compile("\s*Num Local DOFs:\s*([\d]+)")

MfdRE = re.compile("^Finite difference Jacobian \(user-defined state\)")
MhcRE = re.compile("^Hand-coded Jacobian \(user-defined state\)")
MdiffRE = re.compile("^Hand-coded minus finite difference Jacobian \(user-defined state\)")

rowRE = re.compile("row ([\d]+): ")
valRE = re.compile(" \(([\d]+), ([+-.e\d]+)\)")


# Get the real path of jacobian analyzer
if(os.path.islink(sys.argv[0])):
  pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
else:
  pathname = os.path.dirname(sys.argv[0])
  pathname = os.path.abspath(pathname)


# Borrowed from Peacock
def recursiveFindFile(current_path, p, executable):
  if not os.path.exists(current_path):
    return None

  files = os.listdir(current_path)

  split = current_path.split('/')

  if len(split) > 2 and split[-2] == 'problems':
    # if we're in the "problems" directory... hop over to this application's directory instead
    the_file = recursiveFindFile('/'.join(split[:-2]) + '/' + split[-1], p, executable)
    # Still didn't find it, we must keep looking up this path so fall through here
    if the_file != None:
      return the_file

  for afile in files:
    if p.match(afile) and ((not executable) or os.access(current_path+'/'+afile, os.X_OK)):
      return current_path + '/' + afile

  up_one = os.path.dirname(current_path)

  if current_path != '/':
    return recursiveFindFile(up_one, p, executable)

  return None


# Borrowed from Peacock
def findExecutable(executable_option, method_option):
  if executable_option and os.path.exists(executable_option):
    return executable_option
  else:
    # search up directories until we find an executable, starting with the current directory

    method = 'opt' # Find the optimized binary by default

    if 'METHOD' in os.environ:
      method = os.environ['METHOD']

    if method_option:
      method = method_option

    p = re.compile('.+-'+method+'$')

    executable = recursiveFindFile(os.getcwd(), p, True)

    if not executable:
      print 'Executable not found! Try specifying it using -e'
      sys.exit(1)

    return executable


def analyze(numvars, nlvars, dofs, Mfd, Mhc, Mdiff) :
  fd   = np.zeros((numvars, numvars))
  hc   = np.zeros((numvars, numvars))
  norm = np.zeros((numvars, numvars))

  for i in range(dofs) :
    for j in range(dofs) :
      if abs(Mfd[i][j]) > 1e60 or abs(Mhc[i][j]) > 1e60:
        fd  [i % numvars][j % numvars] += 1e10
        norm[i % numvars][j % numvars] += 1e20
        continue
      else :
        fd  [i % numvars][j % numvars] += Mfd[i][j]**2
      hc  [i % numvars][j % numvars] += Mhc[i][j]**2
      norm[i % numvars][j % numvars] += Mdiff[i][j]**2

  fd = fd**0.5
  hc = hc**0.5
  norm = norm**0.5
  all_good = True

  e = 1e-4

  for i in range(numvars) :
    printed = False

    for j in range(numvars) :
      if norm[i][j] > e*fd[i][j] :
        if not printed :
          print "\nKernel for variable '%s':" % nlvars[i]
          printed = True
          all_good = False

        if hc[i][j] == 0.0 :
          problem = "needs to be implemented"
        elif fd[i][j] == 0.0 :
          problem = "should just return  zero"
        else :
          err = math.fabs((hc[i][j]-fd[i][j])/fd[i][j])*100.0
          if err > 20.0 :
            problem = "is wrong (off by %.1f %%)" % err
          elif err > 5.0 :
            problem = "is questionable (off by %.2f %%)" % err
          elif err > 1.0 :
            problem = "is inexact (off by %.3f %%)" % err
          else :
            problem = "is slightly off (by %f %%)" % err

        if i == j :
          print "  (%d,%d) On-diagonal Jacobian %s" % (i, j, problem)
        else :
          print "  (%d,%d) Off-diagonal Jacobian for variable '%s' %s" % (i, j, nlvars[j], problem)

  if all_good :
    print "No errors detected. :-)"


#
# Simple state machine parser for the MOOSE output
#
def parseOutput(output) :
  state = 1
  for line in output.split('\n'):

    #
    # Read in MOOSE startup info, such as variables and DOFs
    #

    if state == 1 :
      # looking fornon linear system header
      m = nlsRE.match(line)
      if m :
        state = 2
        continue

    if state == 2 :
      # look for DOF numbers
      m = dofRE.match(line)
      if m :
        dofs = int(m.group(1))
        continue
      m = ldofRE.match(line)
      if m :
        ldofs = int(m.group(1))
        continue

      # looking for variables list
      m = varlistFullRE.match(line)
      if m :
        nlvarlist = m.group(1)
        nlvars = varRE.findall(nlvarlist)
        state = 4
        continue

      m = varlistStartRE.match(line)
      if m :
        nlvarlist = m.group(1)
        nlvars = varRE.findall(nlvarlist)
        state = 3
        continue

    if state == 3 :
      # continue looking for variables in a multi line list
      m = varlistEndRE.match(line)
      if m :
        nlvarlist = m.group(1)
        nlvars.extend(varRE.findall(nlvarlist))
        state = 4
        continue

      m = varlistContRE.match(line)
      if m :
        nlvarlist = m.group(1)
        nlvars.extend(varRE.findall(nlvarlist))
        continue

    #
    # Initialization
    #

    if state == 4 :
      if dofs != ldofs :
        print "run in serial for debugging"
        sys.exit(1)

      numvars = len(nlvars)
      state = 5

    #
    # Start of a new set of matrices
    #

    if state == 5 :
      Mfd = np.zeros((dofs, dofs))
      Mhc = np.zeros((dofs, dofs))
      Mdiff = np.zeros((dofs, dofs))
      state = 6

    #
    # Read in PetSc matrices
    #

    if state == 6 :
      m = MfdRE.match(line)
      if m :
        state = 7
        continue

    if state == 7 :
      m = MhcRE.match(line)
      if m :
        state = 8
        continue

    if state == 8 :
      m = MdiffRE.match(line)
      if m :
        state = 9
        continue

    # read data
    if state >= 7 and state <= 9 :
      m = rowRE.match(line)
      vals = valRE.findall(line)
      if m :
        row = int(m.group(1))

        for pair in vals :
          if state == 7 :
            Mfd[row, int(pair[0])] = float(pair[1])
          if state == 8 :
            Mhc[row, int(pair[0])] = float(pair[1])
          if state == 9 :
            Mdiff[row, int(pair[0])] = float(pair[1])

        if state == 9 and row+1 == dofs :
          state = 5

          analyze(numvars, nlvars, dofs, Mfd, Mhc, Mdiff)

          # theoretically we could have multiple steps to analyze in the output
          continue


if __name__ == '__main__':
  usage = "Usage: %prog [options] [input_file]"
  description = "Note: You can directly supply an input file without specifying any options.  The correct thing will automatically happen."
  parser = OptionParser(usage=usage, description=description)

  parser.add_option("-e", "--executable", dest="executable",
                    help="The executable you would like to build an input file for.  If not supplied an executable will be searched for.  The searched for executable will default to the optimized version of the executable (if available).")

  parser.add_option("-i", "--input-file", dest="input_file",
                    help="Input file you would like to open debug the jacobians on.")

  parser.add_option("-m", "--method", dest="method",
                    help="Pass either opt, dbg or devel.  Works the same as setting the $METHOD environment variable.")

  parser.add_option("-r", "--resize-mesh", dest="resize_mesh", action="store_true", help="Perform resizing of generated meshs (to speed up the testing).")

  parser.add_option("-s", "--mesh-size", dest="mesh_size", default=1, type="int", help="Set the mesh dimensions to this number of elements along each dimension (requires -r option).")

  (options, args) = parser.parse_args()

  for arg in args:
    if '.i' in arg:
      options.input_file = arg

  if options.input_file is None :
    print 'Please specify an input file.'
    sys.exit(1)

  executable = findExecutable(options.executable, options.method)

  print 'Running input with executable %s ...\n' % executable

  mooseparams = [executable, '-i', options.input_file, '-snes_type', 'test', '-snes_test_display', '-mat_fd_type', 'ds', 'Executioner/solve_type=NEWTON']
  if options.resize_mesh :
    mooseparams.extend(['Mesh/nx=%d' % options.mesh_size, 'Mesh/ny=%d' % options.mesh_size, 'Mesh/nz=%d' % options.mesh_size])

  try:
    data = subprocess.Popen(mooseparams, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
  except:
    print 'Peacock: Error executing moose based application\n'
    sys.exit(1)

  parseOutput(data)

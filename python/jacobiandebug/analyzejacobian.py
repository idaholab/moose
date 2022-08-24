#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sys
import os
import re
import math
import json
import shutil
import numpy as np
import subprocess

from optparse import OptionParser
sys.path.append(os.path.join(os.path.dirname(__file__), '..'))
from TestHarness import util


# These kernels have guaranteed correct Jacobians
whitelisted_kernels = ['Diffusion', 'TimeDerivative']

# Get the real path of jacobian analyzer
if(os.path.islink(sys.argv[0])):
    pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
else:
    pathname = os.path.dirname(sys.argv[0])
    pathname = os.path.abspath(pathname)

class REStruct(object):
    def __init__(self, new_petsc):
        self.MfdRE = "Finite[ -]difference Jacobian"
        self.MhcRE = "Hand-coded Jacobian"
        self.MdiffRE = "Hand-coded minus finite[ -]difference Jacobian"
        self.rowRE = re.compile("row (\d+):")
        self.valRE = re.compile(" \((\d+), ([+.e\d-]+)\)")
        if not new_petsc:
            additional_string = " \(user-defined state\)"
        else:
            additional_string = ""
        self.MfdRE, self.MhcRE, self.MdiffRE = (re.compile(item + additional_string) for item in
                                                (self.MfdRE, self.MhcRE, self.MdiffRE))
        self.new_petsc = new_petsc

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
    elif shutil.which(executable_option):
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
            print('Executable not found! Try specifying it using -e')
            sys.exit(1)

        return executable

#
# v
#   sd1  kern1 kern2
#   sd2  kern1
#
# u
#   sd1  kern3

def analyze(dofdata, Mfd, Mhc, Mdiff) :
    global options

    if options.only is None :
        only = None
    else :
        only = options.only.split(' ')

    diagonal_only = options.diagonal_only

    nlvars = [var['name'] for var in dofdata['vars']]

    # build analysis blocks (for now: one block per variable)
    blocks = []
    for var in dofdata['vars'] :
        blockdofs = []
        for subdomain in var['subdomains'] :
            blockdofs.extend(subdomain['dofs'])
        blocks.append(blockdofs)
    nblocks = len(blocks)

    # analysis results
    fd   = np.zeros((nblocks, nblocks))
    hc   = np.zeros((nblocks, nblocks))
    norm = np.zeros((nblocks, nblocks))

    # prepare block norms
    for i in range(nblocks) :
        for j in range(nblocks) :

            # iterate over all DOFs in the current block and compute the block norm

            for di in blocks[i] :
                for dj in blocks[j] :
                    if abs(Mfd[di][dj]) > 1e60 or abs(Mhc[di][dj]) > 1e60:
                        fd  [i][j] += 1e10
                        norm[i][j] += 1e20
                        continue
                    else :
                        fd  [i][j] += Mfd[di][dj]**2
                    hc  [i][j] += Mhc[di][dj]**2
                    norm[i][j] += Mdiff[di][dj]**2

    fd = fd**0.5
    hc = hc**0.5
    norm = norm**0.5
    all_good = True

    rel_tol = options.rel_tol
    abs_tol = options.abs_tol

    for i in range(nblocks) :
        printed = False

        for j in range(nblocks) :

            if only is not None and ('%s,%s' % (nlvars[i], nlvars[j])) not in only :
                continue
            if i != j and diagonal_only :
                continue

            if norm[i][j] > rel_tol * fd[i][j] and norm[i][j] > abs_tol:
                if not printed :
                    print("\nKernel for variable '%s':" % nlvars[i])
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
                    print("  (%d,%d) On-diagonal Jacobian %s" % (i, j, problem))
                else :
                    print("  (%d,%d) Off-diagonal Jacobian for variable '%s' %s" % (i, j, nlvars[j], problem))

    if all_good :
        print("No errors detected. :-)")


# output parsed (but not processed) jacobian matric data in gnuplot's nonuniform matrix format
def saveMatrixToFile(M, dofs, filename) :
    file = open(filename, "w")
    for i in range(dofs) :
        for j in range(dofs) :
            file.write("%d %d %f\n" % (i, j, M[i][j]))
        file.write("\n")


#
# Simple state machine parser for the MOOSE output
#
def parseOutput(output, dofdata, re_struct) :
    global options

    if re_struct.new_petsc:
        first_re = re_struct.MhcRE
        second_re = re_struct.MfdRE
    else:
        first_re = re_struct.MfdRE
        second_re = re_struct.MhcRE

    write_matrices = options.write_matrices
    dofs = dofdata['ndof']
    success = 0

    state = 0
    for line in output.split('\n'):
        #print state, line

        #
        # Read in PetSc matrices
        #

        if state == 0 :
            Mfd = np.zeros((dofs, dofs))
            Mhc = np.zeros((dofs, dofs))
            if re_struct.new_petsc:
                first_matrix = Mhc
                second_matrix = Mfd
            else:
                first_matrix = Mfd
                second_matrix = Mhc
            Mdiff = np.zeros((dofs, dofs))
            state = 1

        if state == 1 :
            m = first_re.search(line)
            if m :
                state = 2
                continue

        if state == 2 :
            m = second_re.search(line)
            if m :
                state = 3
                continue

        if state == 3 :
            m = re_struct.MdiffRE.search(line)
            if m :
                state = 4
                continue

        # read data
        if state >= 2 and state <= 4 :
            m = re_struct.rowRE.match(line)
            vals = re_struct.valRE.findall(line)
            if m :
                row = int(m.group(1))

                for pair in vals :
                    if state == 2 :
                        first_matrix[row, int(pair[0])] = float(pair[1])
                    if state == 3 :
                        second_matrix[row, int(pair[0])] = float(pair[1])
                    if state == 4 :
                        Mdiff[row, int(pair[0])] = float(pair[1])

                if state == 4 and row+1 == dofs :
                    state = 0

                    analyze(dofdata, Mfd, Mhc, Mdiff)
                    success += 1

                    # dump parsed matrices in gnuplottable format
                    if write_matrices :
                        saveMatrixToFile(Mfd, dofs, "jacobian_finite_differenced.dat")
                        saveMatrixToFile(Mhc, dofs, "jacobian_hand_coded.dat")
                        saveMatrixToFile(Mdiff, dofs, "jacobians_diffed.dat")

                    # theoretically we could have multiple steps to analyze in the output
                    continue

    return success



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
    parser.add_option("-s", "--mesh-size", dest="mesh_size", default=1, type="int", help="Set the mesh dimensions to this number of elements along each dimension (defaults to 1, requires -r option).")

    parser.add_option("-o", "--only", dest="only", help="Test specified Jacobians only (space separated list of comma separated variable pairs).")
    parser.add_option("-D", "--on-diagonal-only", dest="diagonal_only", action="store_true", help="Test on-diagonal Jacobians only.")

    parser.add_option("-d", "--debug", dest="debug", action="store_true", help="Output the command line used to run the application.")
    parser.add_option("-w", "--write-matrices", dest="write_matrices", action="store_true", help="Output the Jacobian matrices in gnuplot format.")
    parser.add_option("-n", "--no-auto-options", dest="noauto", action="store_true", help="Do not add automatic options to the invocation of the moose based application. Requres a specially prepared input file for debugging.")
    parser.add_option("--rel-tol", dest="rel_tol", default=1e-4, type="float", help="The relative tolerance on the Jacobian elements between the hand-coded and those evaluated with finite difference.")
    parser.add_option("--abs-tol", dest="abs_tol", default=1e-12, type="float", help="The absolute tolerance on the Jacobian elements between the hand-coded and those evaluated with finite difference.")
    parser.add_option('--cli-args', type=str, dest='cli_args', help='Append the following list of arguments to the command line (Encapsulate the command in quotes)')

    (options, args) = parser.parse_args()

    for arg in args:
        if arg[-2:] == '.i':
            options.input_file = arg

    if options.input_file is None :
        print('Please specify an input file.')
        sys.exit(1)

    executable = findExecutable(options.executable, options.method)

    basename = options.input_file[0:-2]
    dofoutname = 'analyzerdofmap'

    # common arguments for both debugging and dofmapping
    moosebaseparams = [executable, '-i', options.input_file ]
    if options.resize_mesh :
        moosebaseparams.extend(['Mesh/nx=%d' % options.mesh_size, 'Mesh/ny=%d' % options.mesh_size, 'Mesh/nz=%d' % options.mesh_size])


    # run to dump DOFs (this does not happen during the debug step)
    dofmapfilename = basename + '_' + dofoutname + '.json'
    if not options.noauto :
        mooseparams = moosebaseparams[:]
        mooseparams.extend(['Problem/solve=false', 'BCs/active=', 'Outputs/' + dofoutname+ '/type=DOFMap', 'Outputs/active=' + dofoutname, 'Outputs/file_base=' + basename + '_' + dofoutname, '--no-gdb-backtrace'])
        if options.cli_args != None:
            mooseparams.extend([options.cli_args])
        if options.debug :
            print("Running\n%s\n" % " ".join(mooseparams))
        try:
            child = subprocess.Popen(mooseparams, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
            data = child.communicate()[0]
            if child.wait() :
                print("Application quit with non-zero return code")
                print(data)
                sys.exit(1)
        except:
            print('Error executing moose based application to gather DOF map\n')
            sys.exit(1)
    else :
        print("Running without automatic options DOF map '%s' will not be generated automatically!" % dofmapfilename)


    # load and decode the DOF map data (for now we only care about one frame)
    with open (dofmapfilename, "rt") as myfile :
        dofjson = myfile.readlines()
    dofdata = json.loads(dofjson[0].rstrip('\n'))
    if options.debug :
        print("DOF map output:\n%s\n" % dofdata)


    # for every DOF get the list of kernels contributing to it
    dofkernels = [[] for i in range(dofdata['ndof'])]
    kerneltypes = {}
    for var in dofdata['vars'] :
        for subdomain in var['subdomains'] :
            kernels = [kernel for kernel in subdomain['kernels']]

            # create lookup table from kernel name to kernel type
            for kernel in kernels :
                kerneltypes[kernel['name']] = kernel['type']

            # list of active kernels contributing to a DOF
            for dof in subdomain['dofs'] :
                dofkernels[dof].extend([kernel['name'] for kernel in kernels if not kernel['name'] in dofkernels[dof]])

    # get all unique kernel combinations occurring on the DOFs
    combination_dofs = {}
    for dof, kernels in enumerate(dofkernels) :
        kernels.sort()
        idx = tuple(kernels)
        if idx in combination_dofs :
            combination_dofs[idx].append(dof)
        else :
            combination_dofs[idx] = [dof]

    #combinations = []
    #for kernels in combination_dofs :
    #   print kernels

    moose_dir = os.environ.get('MOOSE_DIR',
                               os.path.abspath(os.path.join(os.path.dirname(os.path.realpath(__file__)),
                                                            '../..')))
    if os.environ.get("LIBMESH_DIR"):
        libmesh_dir = os.environ['LIBMESH_DIR']
    else:
        libmesh_dir = os.path.join(moose_dir, 'libmesh', 'installed')
    new_petsc = list(map(int, util.getPetscVersion(libmesh_dir).split("."))) >= [3, 9]

    # build the parameter list for the jacobian debug run
    mooseparams = moosebaseparams[:]
    if not options.noauto :
        if new_petsc:
            petsc_test_options = ['-snes_test_jacobian', '-snes_test_jacobian_view', '-snes_type', 'ksponly',
                                  '-ksp_type', 'preonly', '-pc_type', 'none', '-snes_convergence_test', 'skip',
                                  '-snes_test_jacobian_display_threshold', '1e-10']
        else:
            petsc_test_options = ['-snes_type', 'test', '-snes_test_display']

        mooseparams.extend(petsc_test_options + ['-mat_fd_type', 'ds',
                                                 'BCs/active=', 'Outputs/exodus=false', 'Outputs/csv=false',
                                                 'Outputs/active=', 'Executioner/solve_type=NEWTON', '--no-gdb-backtrace'])
    if options.cli_args != None:
        mooseparams.extend([options.cli_args])

    if options.debug :
        print("Running\n%s\n" % " ".join(mooseparams))
    else :
        print('Running input with executable %s ...\n' % executable)

    # run debug process to gather jacobian data
    try:
        child = subprocess.Popen(mooseparams, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        data = child.communicate()[0].decode("utf-8")
        # analyze return code
        if child.returncode == -11 :
            print("The moose application crashed with a segmentation fault (try recompiling)")
            sys.exit(1)
        elif (not new_petsc and child.returncode != 1) or (new_petsc and child.returncode != 0):
            print(data)
            print("Application quit with an unexpected return code")
            sys.exit(1)
    except:
        print('Error executing moose based application\n')
        sys.exit(1)

    re_struct = REStruct(new_petsc)
    # parse the raw output, which contains the PETSc debug information
    if parseOutput(data, dofdata, re_struct) == 0:
        print(data)
        print('Error executing moose based application\n')
        sys.exit(1)

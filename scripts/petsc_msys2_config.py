#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

configure_options = [
    '--with-metis=1',
    '--with-parmetis=1',
#    '--with-mpi-include=/mingw64/include',
#    '--with-mpi-lib=/mingw64/lib/libmsmpi.a',
    '--with-ar=/usr/bin/ar' ,
    '--with-shared-libraries=0',
    '--with-debugging=0',
    '--with-sowing=0',
    '--with-visibility=0',
    '--with-fortran-bindings=0',
    '--download-hypre=1',
    '--download-hypre-configure-arguments="--host=x84-mingw64"',
    '--prefix=/usr/local/petsc',
    'FOPTFLAGS=-O3 -fno-range-check',
  ]

if __name__ == '__main__':
    import sys,os
    if sys.platform != 'msys':
        print("Please run this script with an MSYS2 python (e.g. python2.7 %s)" % sys.argv[0])
        sys.exit(1)

    dirname = os.path.dirname(__file__)
    os.chdir(os.path.abspath(os.path.join(dirname, '../petsc')))
    sys.path.insert(0, os.path.abspath('config'))

    import configure
    configure.petsc_configure(configure_options)

#!/usr/bin/env python
# /*************************************************/
# /*           DO NOT MODIFY THIS HEADER           */
# /*                                               */
# /*                     BISON                     */
# /*                                               */
# /*    (c) 2015 Battelle Energy Alliance, LLC     */
# /*            ALL RIGHTS RESERVED                */
# /*                                               */
# /*   Prepared by Battelle Energy Alliance, LLC   */
# /*     Under Contract No. DE-AC07-05ID14517      */
# /*     With the U. S. Department of Energy       */
# /*                                               */
# /*     See COPYRIGHT for full restrictions       */
# /*************************************************/
import sys
import os

# Locate MOOSE directory
os.chdir(os.path.realpath(os.path.dirname(__file__)))
MOOSE_DIR = os.getenv('MOOSE_DIR', os.path.join(os.getcwd(), '..', 'moose'))

if not os.path.exists(MOOSE_DIR):
    MOOSE_DIR = os.getenv(
        'MOOSE_DIR', os.path.join(os.getcwd(), '..', '..', '..', 'moose'))
if not os.path.exists(os.path.join(MOOSE_DIR, 'libmesh')):
    MOOSE_DIR = os.path.join(os.getcwd(), '..', '..', 'moose')
if not os.path.exists(MOOSE_DIR):
    raise Exception(
        'Failed to locate MOOSE, specify the MOOSE_DIR environment variable.')
os.environ['MOOSE_DIR'] = MOOSE_DIR

# Locate IAPWS95 submodule
if 'IAPWS95_DIR' not in os.environ:
    os.environ['IAPWS95_DIR'] = os.path.abspath(
        os.path.join(os.path.dirname(__name__), '..', 'iapws95'))

# Append MOOSE python directory
MOOSE_PYTHON_DIR = os.path.join(MOOSE_DIR, 'python')
if MOOSE_PYTHON_DIR not in sys.path:
    sys.path.append(MOOSE_PYTHON_DIR)

from MooseDocs import main
if __name__ == '__main__':
    sys.exit(main.run())

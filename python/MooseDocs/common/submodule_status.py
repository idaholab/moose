#pylint: disable=missing-docstring
####################################################################################################
#                                    DO NOT MODIFY THIS HEADER                                     #
#                   MOOSE - Multiphysics Object Oriented Simulation Environment                    #
#                                                                                                  #
#                              (c) 2010 Battelle Energy Alliance, LLC                              #
#                                       ALL RIGHTS RESERVED                                        #
#                                                                                                  #
#                            Prepared by Battelle Energy Alliance, LLC                             #
#                               Under Contract No. DE-AC07-05ID14517                               #
#                               With the U. S. Department of Energy                                #
#                                                                                                  #
#                               See COPYRIGHT for full restrictions                                #
####################################################################################################
#pylint: enable=missing-docstring
import subprocess
import re
import MooseDocs

def submodule_status(working_dir=MooseDocs.MOOSE_DIR):
    """
    Return the status of each of the git submodule(s).
    """
    out = dict()
    result = subprocess.check_output(['git', 'submodule', 'status'], cwd=working_dir)
    regex = re.compile(r'(?P<status>[\s\-\+U])(?P<sha1>[a-f0-9]{40})\s(?P<name>.*?)\s')
    for match in regex.finditer(result):
        out[match.group('name')] = match.group('status')
    return out

#!/usr/bin/env python
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

import unittest
import mooseutils

from MooseDocs.common import Builder
from MooseDocs.testing import LogTestCase

class TestBuilder(LogTestCase):
    """
    Tests Builder base class
    """
    def testErrors(self):
        builder = Builder(None, None)
        with self.assertRaises(mooseutils.MooseException) as e:
            list(builder)
        self.assertIn("The 'init' method must be called prior to iterating", str(e.exception))

        with self.assertRaises(mooseutils.MooseException) as e:
            builder.count()
        self.assertIn("The 'init' method must be called prior to count", str(e.exception))

        with self.assertRaises(mooseutils.MooseException) as e:
            builder.build()
        self.assertIn("The 'init' method must be called prior to build", str(e.exception))

        with self.assertRaises(NotImplementedError) as e:
            builder.buildNodes()
        self.assertIn("The 'buildNodes' method must be defined", str(e.exception))

if __name__ == '__main__':
    unittest.main(verbosity=2)

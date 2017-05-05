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
from MooseDocs.testing import MarkdownTestCase

class TestMooseObjectSyntax(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.app_syntax']

    def testDescription(self):
        md = '!description /Adaptivity/Markers/BoxMarker'
        self.assertConvert('test_description.html', md)

    def testDescriptionOptions(self):
        md = '!description /Adaptivity/Markers/BoxMarker float=right width=42%'
        self.assertConvert('test_description_options.html', md)

    def testDescriptionError(self):
        md = '!description /Not/A/Real/Object'
        self.assertConvert('test_description_error.html', md)

    def testParameters(self):
        md = '!parameters /Adaptivity/Markers/BoxMarker'
        html = self.convert(md)
        self.assertIn('<h2>Input Parameters</h2>', html)
        self.assertIn('<h3>Required Parameters</h3>', html)
        self.assertIn('<div class="moose-parameter-description">How to mark elements outside ' \
                      'the box.</div>', html)
        self.assertIn('<div class="moose-parameter-default">Default: None</div>', html)
        self.assertIn('<div class="moose-parameter-type">Type: <code>MooseEnum</code></div>', html)
        self.assertIn('<h3>Advanced Parameters</h3>', html)
        self.assertIn('<div class="moose-parameter-description">Adds user-defined labels for ' \
                      'accessing object parameters via control logic.</div>', html)

    def testParametersOptions(self):
        md = '!parameters /Kernels/Diffusion float=right width=42%'
        html = self.convert(md)
        self.assertIn('<div style="float:right;width:42%;">', html)

    def testInputFiles(self):
        md = '!inputfiles /Adaptivity/Markers/BoxMarker'
        html = self.convert(md)
        self.assertIn('<div class="section scrollspy" id="#input-files">', html)
        self.assertIn('<h2>Input Files</h2>', html)
        self.assertIn('<h3>Tests</h3>', html)
        self.assertIn('<ul style="max-height:350px;overflow-y:Scroll">', html)
        self.assertIn('<li><a href="https://github.com/idaholab/moose/blob/master/test/tests/' \
                      'adaptivity/initial_adapt/initial_adapt.i">test/tests/adaptivity/' \
                      'initial_adapt/initial_adapt.i</a></li>', html)

    def testChildObjects(self):
        md = '!childobjects /Kernels/Diffusion'
        html = self.convert(md)
        self.assertIn('<div class="section scrollspy" id="#child-objects">', html)
        self.assertIn('<h2>Child Objects</h2>', html)
        self.assertIn('<h3>Tutorials</h3>', html)
        self.assertIn('<ul style="max-height:350px;overflow-y:Scroll">', html)
        self.assertIn('<li><a href="https://github.com/idaholab/moose/blob/master/tutorials' \
                      '/darcy_thermo_mech/step02_darcy_pressure/include/kernels/DarcyPressure.h">' \
                      'tutorials/darcy_thermo_mech/step02_darcy_pressure/include/kernels/' \
                      'DarcyPressure.h</a></li>', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)

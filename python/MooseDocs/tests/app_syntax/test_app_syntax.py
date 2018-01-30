#!/usr/bin/env python
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
from MooseDocs.testing import MarkdownTestCase

class TestAppSyntax(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.app_syntax']

    def testParameters(self):
        md = '!syntax parameters /Adaptivity/Markers/BoxMarker'
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
        md = '!syntax parameters /Kernels/Diffusion float=right width=42%'
        html = self.convert(md)
        self.assertIn('<div style="float:right;width:42%;">', html)

    def testParametersDisableObjects(self):
        md = '!syntax parameters /BCs/Pressure objects=False'
        self.parser.inlinePatterns['moose_syntax_parameters'].clearCache()
        self.convert(md)
        self.assertInLogError('Failed to locate Action')

    def testParametersDisableActions(self):
        md = '!syntax parameters /BCs/Pressure actions=False'
        html = self.convert(md)

        content = '<div class="moose-parameter-name">variable'
        self.assertIn(content, html)

        content = '<div class="moose-parameter-names">disp_x'
        self.assertNotIn(content, html)

    def testParametersSyntaxError(self):
        md = '!syntax parameters /Not/Valid/Syntax'
        self.convert(md)
        self.assertInLogError('Failed to locate Action or MooseObject')
        self.assertInLogError('syntax parameters /Not/Valid/Syntax')

    def testDescription(self):
        md = '!syntax description /Adaptivity/Markers/BoxMarker'
        html = self.convert(md)
        self.assertIn("Marks the region inside and outside of a 'box' domain for refinement or "
                      "coarsening.", html)

    def testDescriptionOptions(self):
        md = '!syntax description /Adaptivity/Markers/BoxMarker float=right width=42%'
        html = self.convert(md)
        self.assertIn('<p style="float:right;width:42%;">', html)

    def testDescriptionError(self):
        md = '!syntax description /Not/A/Real/Object'
        self.convert(md)
        self.assertInLogError('Failed to locate Action or MooseObject')
        self.assertInLogError('syntax description /Not/A/Real/Object')

    def testDescriptionDisableObjects(self):
        md = '!syntax description /BCs/Pressure objects=False'
        self.parser.inlinePatterns['moose_syntax_description'].clearCache()
        self.convert(md)
        self.assertInLogError('Failed to locate Action')

    def testDescriptionDisableActionsError(self):
        md = '!syntax description /BCs/Pressure actions=False'
        html = self.convert(md)
        self.assertIn("Applies a pressure on a given boundary in a given direction", html)

    def testDesriptionAction(self):
        md = '!syntax description /BCs/Pressure/PressureAction'
        html = self.convert(md)
        self.assertIn("Set up Pressure boundary conditions", html)

    def testChildObjects(self):
        md = '!syntax children /Kernels/Diffusion'
        html = self.convert(md)
        self.assertIn('<div class="section scrollspy" id="#child-objects">', html)
        self.assertIn('<h2>Child Objects</h2>', html)
        self.assertIn('<h3>Tutorials</h3>', html)
        self.assertIn('<ul style="max-height:350px;overflow-y:Scroll">', html)
        self.assertIn('<li><a href="https://github.com/idaholab/moose/blob/master/tutorials' \
                      '/darcy_thermo_mech/step02_darcy_pressure/include/kernels/DarcyPressure.h">' \
                      'tutorials/darcy_thermo_mech/step02_darcy_pressure/include/kernels/' \
                      'DarcyPressure.h</a></li>', html)

    def testChildObjectTitle(self):
        md = '!syntax children /Kernels/Diffusion title=Foo Bar title_level=4'
        html = self.convert(md)
        self.assertIn('<div id="#foo-bar">', html)
        self.assertIn('<h4>Foo Bar</h4>', html)
        self.assertIn('<h5>Tutorials</h5>', html)

    def testChildObjectTitleNone(self):
        md = '!syntax children /Kernels/Diffusion title=None'
        html = self.convert(md)
        self.assertNotIn('<h2>', html)

    def testInputFiles(self):
        md = '!syntax inputs /Adaptivity/Markers/BoxMarker'
        html = self.convert(md)
        self.assertIn('<div class="section scrollspy" id="#input-files">', html)
        self.assertIn('<h2>Input Files</h2>', html)
        self.assertIn('<h3>Tests</h3>', html)
        self.assertIn('<ul style="max-height:350px;overflow-y:Scroll">', html)
        self.assertIn('<li><a href="https://github.com/idaholab/moose/blob/master/test/tests/' \
                      'adaptivity/initial_adapt/initial_adapt.i">test/tests/adaptivity/' \
                      'initial_adapt/initial_adapt.i</a></li>', html)

    def testObjects(self):
        md = '!syntax objects /Adaptivity/Markers'
        html = self.convert(md)
        self.assertIn('<h2>Available Sub-Objects</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Objects' \
                      '</div>', html)

        gold = '<a href="/Adaptivity/Markers/framework/BoxMarker.md">BoxMarker</a>'
        self.assertIn(gold, html)

    def testObjectsTitle(self):
        md = '!syntax objects /Adaptivity/Markers title=My Custom Title title_level=3'
        html = self.convert(md)
        self.assertIn('<h3>My Custom Title</h3>', html)

    def testObjectsError(self):
        md = '!syntax objects /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)

    def testActions(self):
        md = '!syntax actions /Adaptivity/Markers'
        html = self.convert(md)
        self.assertIn('<h2>Associated Actions</h2>', html)

        msg = 'href="/Adaptivity/Markers/framework/AddElementalFieldAction.md">' \
              'AddElementalFieldAction</a>'
        self.assertIn(msg, html)

    def testSubSystems(self):
        md = '!syntax subsystems /Adaptivity'
        html = self.convert(md)
        self.assertIn('<h2>Available Sub-Systems</h2>', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Systems</div>',
                      html)
        gold = 'href="/Adaptivity/Indicators/index.md">Indicators</a>'
        self.assertIn(gold, html)

    def testSubSystemsTitle(self):
        md = '!syntax subsystems /Adaptivity title=My Custom Title'
        html = self.convert(md)
        self.assertIn('<h2>My Custom Title</h2>', html)

    def testSubSystemsError(self):
        md = '!syntax subsystems /Not/A/Valid/System'
        html = self.convert(md)
        self.assertIn('<div class="admonition error">', html)

    def testComplete(self):
        md = '!syntax complete'
        html = self.convert(md)
        self.assertIn('<h2 id="adaptivity">Adaptivity', html)
        self.assertIn('<h3 id="adaptivity-markers">Adaptivity/Markers', html)
        self.assertIn('<div class="collapsible-header moose-group-header">Framework Objects</div>',
                      html)
        self.assertIn('Adaptivity/Markers/framework/BoxMarker.md">BoxMarker</a>',
                      html)

    def testCompleteGroup(self):
        md = '!syntax complete groups=level_set'
        html = self.convert(md)
        gold = '<div class="collapsible-header moose-group-header">Framework Objects</div>'
        self.assertNotIn(gold, html)

if __name__ == '__main__':
    unittest.main(verbosity=2)

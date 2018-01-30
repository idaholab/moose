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

class CSVTestCase(MarkdownTestCase):
    EXTENSIONS = ['MooseDocs.extensions.gchart']
    COLS = 'snow_depth_set_1,air_temp_set_1'
    CSV = 'python/peacock/tests/input/white_elephant_jan_2016.csv'

class TestLineChart(CSVTestCase):
    def testDefault(self):
        md = '!chart line columns={} csv={}'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn('google.charts.Line(document.getElementById("moose-google-line-chart-', html)

    def testId(self):
        md = '!chart line id=foo columns={} csv={}'.format(self.COLS, self.CSV)
        html = self.convert(md)
        gold = '<div class="moose-float-div moose-figure-div" ' \
               'data-moose-float-name="Figure" id="foo">'
        self.assertIn(gold, html)

    def testCSV(self):
        md = '!chart line columns={} csv={}'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn("data.addColumn('number', 'snow_depth_set_1')", html)
        self.assertIn("data.addColumn('number', 'air_temp_set_1')", html)
        self.assertIn("data.addRows([[175.0, 9.14], [175.0, 9.32], ", html)

    @unittest.skip("gchart is still in devel and this it too strict currently")
    def testCSVError(self):
        md = '!chart line columns=A'
        self.convert(md)
        self.assertInLogError("The 'csv' setting is required")

    def testTitle(self):
        md = '!chart line columns={} csv={} ' \
             'title=White Elephant SNOTEL Station ' \
             'subtitle=January 2016'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn("title: 'White Elephant", html)
        self.assertIn("subtitle: 'January", html)

    def testWidthHeight(self):
        md = '!chart line columns={} csv={} ' \
             'chart_width=450 chart_height=550'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn("width: 450", html)
        self.assertIn("height: 550", html)

    def testColumnNames(self):
        md = '!chart line columns={} csv={} column_names=DEPTH, TEMP' \
             .format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn("data.addColumn('number', 'DEPTH')", html)
        self.assertIn("data.addColumn('number', 'TEMP')", html)

    def testColumnNameError(self):
        md = '!chart line columns={} csv={} column_names=TIME, DEPTH, TEMP' \
             .format(self.COLS, self.CSV)
        self.convert(md)
        self.assertInLogError("The 'column_names' list must be the same length as 'columns'.")

class TestScatterChart(CSVTestCase):
    def testDefault(self):
        md = '!chart scatter columns={} csv={}'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn('google.charts.Scatter(document.getElementById("moose-google-scatter-chart-',
                      html)

class TestDiffScatterChart(CSVTestCase):
    CSV = 'python/test_files/old_faithful.csv'
    COLS = 'duration,interval'
    def testDefault(self):
        md = '!chart diffscatter columns={} csv={}'.format(self.COLS, self.CSV)
        html = self.convert(md)
        self.assertIn('var diff_data = chart.computeDiff(data, gold)', html)

if __name__ == '__main__':
    unittest.main(verbosity=2)

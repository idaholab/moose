#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
#pylint: enable=missing-docstring

import os
import logging

import pandas
import jinja2

from markdown.util import etree
from markdown.inlinepatterns import Pattern

import MooseDocs
from MooseDocs.common import nodes
from MooseMarkdownExtension import MooseMarkdownExtension
from MooseMarkdownCommon import MooseMarkdownCommon

LOG = logging.getLogger(__name__)

class GoogleChartExtension(MooseMarkdownExtension):
    """
    Adds support for google charts.
    """

    @staticmethod
    def defaultConfig():
        """GoogleChartExtension configuration."""
        config = MooseMarkdownExtension.defaultConfig()
        return config

    def extendMarkdown(self, md, md_globals):
        """
        Adds eqref support for MOOSE flavored markdown.
        """
        md.registerExtension(self)
        config = self.getConfigs()

        md.inlinePatterns.add('moose-line-chart',
                              LineChart(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('moose-scatter-chart',
                              ScatterChart(markdown_instance=md, **config),
                              '_begin')

        md.inlinePatterns.add('moose-diff-scatter-chart',
                              ScatterDiffChart(markdown_instance=md, **config),
                              '_begin')


def makeExtension(*args, **kwargs): #pylint: disable=invalid-name
    """Create GoogleChartExtension"""
    return GoogleChartExtension(*args, **kwargs)

class GoogleChartBase(MooseMarkdownCommon, Pattern):
    """
    Base class for !chart command.
    """
    TEMPLATE = None
    @staticmethod
    def defaultSettings():
        """GoogleChartBase settings."""
        settings = MooseMarkdownCommon.defaultSettings()
        settings['caption'] = (None, "The caption to place after the float heading and number.")
        settings['counter'] = ('figure', "The name of global counter to utilized for numbering.")
        settings['csv'] = (None, "The name of the CSV file to load.")
        return settings

    def __init__(self, markdown_instance=None, **kwargs):
        MooseMarkdownCommon.__init__(self, **kwargs)
        regex = r'^!chart\s+(?P<template>{})(?:$|\s+)(?P<settings>.*)'.format(self.TEMPLATE)
        Pattern.__init__(self, regex, markdown_instance)
        self._csv = dict() # CSV DataFrame cache
        self._count = 0
        self._status = None

    def setStatus(self, message, *args):
        """
        Set the error status message, this should be used in the arguments() and globals() methods.
        """
        self._status = message.format(*args)

    def clearStatus(self):
        """
        Remove any existing error status messages.
        """
        self._status = None

    def arguments(self, settings):
        """
        Method for modifying the template arguments to be applied to the jinja2 templates engine.

        By default all the "settings" from the class are returned as template arguments.

        Args:
            settings[dict]: The class object settings.
        """

        if settings['csv'] is None:
            if isinstance(self.markdown.current, nodes.FileNodeBase):
                self.setStatus("The 'csv' setting is required in {}.",
                               self.markdown.current.filename)
            else:
                self.setStatus("The 'csv' setting is required.")
            settings['data_frame'] = pandas.DataFrame()
        else:
            settings['data_frame'] = self._readCSV(os.path.join(MooseDocs.ROOT_DIR,
                                                                settings['csv']))
        return settings

    def globals(self, env):
        """
        Defines global template functions. (virtual)

        Args:
            env[jinja2.Environment]: Template object for adding global functions.
        """
        pass

    def handleMatch(self, match):
        """
        Creates chart from a chart template.
        """
        # Extract settings and template
        template = match.group('template') + '.js'
        settings = self.getSettings(match.group('settings'), legacy_style=False)

        # Create a float element
        div = self.createFloatElement(settings)

        # Create 'chart_id' for linking JS with <div>
        settings['chart_id'] = 'moose-google-{}-chart-{}'.format(self.TEMPLATE, int(self._count))
        self._count += 1

        # Paths to Google Chart template
        paths = [os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'templates', 'gchart'),
                 os.path.join(os.getcwd(), 'templates', 'gchart')]

        # Apply the arguments to the template
        self.clearStatus()
        env = jinja2.Environment(loader=jinja2.FileSystemLoader(paths))
        self.globals(env)
        template = env.get_template(template)
        complete = template.render(**self.arguments(settings))

        if self._status is not None:
            return self.createErrorElement(self._status, title="Google Chart Creation Error",
                                           error=False)

        # Create the <script> tag
        script = etree.SubElement(div, 'script')
        script.set('type', 'text/javascript')
        script.text = self.markdown.htmlStash.store(complete, safe=True)

        # Add the <div> to be replaced with the chart
        el = etree.Element('div')
        el.set('id', settings['chart_id'])
        div.insert(0, el)
        return div

    def _readCSV(self, filename):
        """
        Read the CSV data into a pandas DataFrame.
        """
        if self._csv.get(filename, None) is None:
            try:
                self._csv[filename] = pandas.read_csv(filename)

            except IOError:
                if isinstance(self.markdown.current, nodes.FileNodeBase):
                    self.setStatus("Failed to read CSV file '{}' in chart command of {}.",
                                   filename, self.markdown.current.filename)
                else:
                    self.setStatus("Failed to read CSV file '{}' in chart command.", filename)
                return pandas.DataFrame()

        return self._csv[filename]

class ColumnChartBase(GoogleChartBase):
    """
    Base class for column based chart types (e.g., 'line', 'scatter').
    """
    @staticmethod
    def defaultSettings():
        """LineChart settings."""
        settings = GoogleChartBase.defaultSettings()
        settings['columns'] = ('', "A comma separated list of names defining the columns from the "
                                   "the CSV to extract for plotting in the chart.")
        settings['column_names'] = ('', "A comma separated list of names to associate with each "
                                        "column, the number of names must match the number of "
                                        "columns.")
        settings['title'] = ('', "The chart title.")
        settings['subtitle'] = ('', "The chart sub-title.")
        settings['chart_width'] = (900, "The Google chart width.")
        settings['chart_height'] = (400, "The Google chart height.")
        return settings

    def arguments(self, settings):
        """
        Define template arguments to pass to template.
        """
        settings = super(ColumnChartBase, self).arguments(settings)

        # Update the 'columns' and 'column_names'
        settings['columns'] = [col.strip() for col in settings['columns'].split(',')]
        if settings['column_names']:
            settings['column_names'] = [col.strip() for col in settings['column_names'].split(',')]
        else:
            settings['column_names'] = settings['columns']

        if len(settings['column_names']) != len(settings['columns']):
            LOG.error("The 'column_names' list must be the same length as 'columns'.")
            settings['column_names'] = settings['columns']

        return settings

class LineChart(ColumnChartBase):
    """
    Creates a Google line chart from CSV data.
    """
    TEMPLATE = 'line'

class ScatterChart(ColumnChartBase):
    """
    Creates a Google scatter chart from CSV data.
    """
    TEMPLATE = 'scatter'
    @staticmethod
    def defaultSettings():
        """ScatterChart settings."""
        settings = ColumnChartBase.defaultSettings()
        settings['vaxis_title'] = ('y', "The vertical y-axis title.")
        settings['haxis_title'] = ('x', "The horizontal x-axis title.")
        settings['vaxis_ticks'] = (None, "The vertical x-axis tick marks (default: auto)")
        settings['haxis_ticks'] = (None, "The vertical x-axis tick marks (default: auto)")
        return settings

class ScatterDiffChart(ScatterChart):
    """
    Creates a Google scatter diff chart from CSV data.
    """
    TEMPLATE = 'diffscatter'

    @staticmethod
    def defaultSettings():
        """DiffScatterChart settings"""
        settings = ScatterChart.defaultSettings()
        settings['gold'] = ('', "The gold file to use for comparison, by default the file provided "
                                "in the 'csv' setting is used but with a gold directory prefix.")
        return settings

    def arguments(self, settings):
        """
        Define template arguments for diff scatter chart.
        """
        settings = super(ScatterDiffChart, self).arguments(settings)

        if not settings['gold']:
            base, name = os.path.split(settings['csv'])
            settings['gold'] = os.path.join(base, 'gold', name)

        settings['gold_data_frame'] = self._readCSV(os.path.join(MooseDocs.ROOT_DIR,
                                                                 settings['gold']))
        if settings['gold_data_frame'].empty:
            self.setStatus("The gold file ({}) does not exist or does not contain data.",
                           settings['gold'])

        return settings

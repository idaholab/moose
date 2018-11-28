#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import re
import uuid
import mooseutils
from MooseDocs import common
from MooseDocs.extensions import command, floats
from MooseDocs.base import components, renderers
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return PlotlyExtension(**kwargs)

ScatterToken = tokens.newToken('ScatterToken', data=[], layout=dict())

class PlotlyExtension(command.CommandExtension):
    """
    Extension for creating graphs via Plotly: https://plot.ly
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = (u'Figure', "The caption prefix (e.g., Fig.).")
        return config

    def extend(self, reader, renderer):
        self.requires(command, floats)
        self.addCommand(reader, PlotlyScatter())
        renderer.add('ScatterToken', RenderScatter())

        if isinstance(renderer, renderers.HTMLRenderer):
            renderer.addJavaScript('plotly', "contrib/plotly/plotly.min.js")

class PlotlyScatter(command.CommandComponent):
    """
    Scatter plot.
    """
    COMMAND = 'plot'
    SUBCOMMAND = 'scatter'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['data'] = (None, "Directly supply a list of dict items (i.e., JSON data) to the "
                                  "plotly plot command, see "
                                  "https://plot.ly/javascript/line-and-scatter"
                                  "for additional details.")
        settings['layout'] = ('dict()', "Plotly layout settings for the chart, refer to "
                                        "https://plot.ly/javascript/reference/#layout "
                                        "for available options.")
        settings['filename'] = (None, "The name of a CSV file for extracting data, when used the "
                                "'x' and 'y' fields of the 'data' setting should be replaced by "
                                "column names or numbers.")
        settings.update(floats.caption_settings())
        settings['prefix'] = ('Figure', settings['prefix'][1])
        return settings

    def createToken(self, parent, info, page):

        # Build the JSON data for plotting
        data = self.settings['data']
        if data is None:
            raise common.exceptions.MooseDocsException("The 'data' setting is required.")
        data = eval(data)

        # Use Postprocessor file for data
        filename = self.settings['filename']
        if filename is not None:
            filename = common.check_filenames(filename)
            reader = mooseutils.PostprocessorReader(filename)

            for i, line in enumerate(data):
                data[i]['x'] = reader(line['x']).tolist()
                data[i]['y'] = reader(line['y']).tolist()

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings)
        ScatterToken(flt, data=data, layout=eval(self.settings['layout']))
        if flt.children[0].name == 'Caption':
            cap = flt.children[0]
            cap.parent = None
            cap.parent = flt

        return parent

class PlotlyTemplate(object):
    """
    Helper for loading plotly javascript templates.

    The template can contain markers such as MOOSEDOCS_ID, which will be
    replaced by the supplied "id" key when the __call__ method is used. Any trailing
    underscore from the key is used (e.g., "id_" == "id").
    """
    RE = re.compile(r'MOOSEDOCS_(?P<key>\w+)')
    def __init__(self, name):
        self.__template = common.read(os.path.join(os.path.dirname(__file__),
                                                   'templates',
                                                   name))

    def __call__(self, **kwargs):
        """Replace the markers with the supplied key, value pairs."""
        for key in kwargs:
            if key.endswith('_'):
                kwargs[key[:-1]] = kwargs.pop(key)

        return self.RE.sub(lambda m: self.subFunction(m, kwargs), self.__template)

    @staticmethod
    def subFunction(match, options):
        """Replace the marker with the content in the options dict."""
        key = match.group('key').lower()
        return options[key]

class RenderScatter(components.RenderComponent):
    """Render a plotly scatter plot."""

    TEMPLATE = PlotlyTemplate('scatter.js')
    def createHTML(self, parent, token, page):
        plot_id = unicode(uuid.uuid4())
        content = self.TEMPLATE(id_=plot_id, data=repr(token['data']), layout=repr(token['layout']))
        html.Tag(parent, 'div', id_=plot_id)
        html.Tag(parent, 'script', string=content)

    def createLatex(self, parent, token, page):
        pass

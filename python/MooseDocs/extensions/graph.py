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
import tempfile

try:
    import plotly
    HAVE_PYTHON_PLOTLY = True
except ImportError:
    HAVE_PYTHON_PLOTLY = False

import mooseutils
from .. import common
from ..base import components, renderers
from ..tree import tokens, html, latex
from . import command, floats

def make_extension(**kwargs):
    return GraphExtension(**kwargs)

ScatterToken = tokens.newToken('ScatterToken', data=[], layout=dict())
HistogramToken = tokens.newToken('HistogramToken', data=[], layout=dict())

class GraphExtension(command.CommandExtension):
    """
    Extension for creating graphs via Plotly: https://plot.ly
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Figure', "The caption prefix (e.g., Fig.).")
        config['draft'] = (False, "Enable draft mode for LaTeX output.")
        return config

    def extend(self, reader, renderer):
        self.requires(command, floats)
        self.addCommand(reader, GraphScatter())
        self.addCommand(reader, GraphHistogram())

        renderer.add('ScatterToken', RenderScatter())
        renderer.add('HistogramToken', RenderHistogram())

        if isinstance(renderer, renderers.LatexRenderer):
            if not HAVE_PYTHON_PLOTLY and not self.get('draft'):
                self.update(draft=True)
                renderer.addPackage('draftfigure',
                                        content="{Draft mode enabled, plotly package failed to load, "
                                                "install with 'conda install plolty plotly-orca'}")
            elif self.get('draft'):
                renderer.addPackage('draftfigure',
                                    content="{Draft mode enabled in the graph extension.}")

            renderer.addPackage('graphicx')

    def postTokenize(self, page, ast):
        if common.has_tokens(ast, 'ScatterToken', 'HistogramToken'):
            renderer = self.translator.renderer
            if isinstance(renderer, renderers.HTMLRenderer):
                renderer.addJavaScript('plotly', "contrib/plotly/plotly.min.js", page, head=True)

class GraphScatter(command.CommandComponent):
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

    def createToken(self, parent, info, page, settings):

        # Build the JSON data for plotting
        data = settings['data']
        if data is None:
            raise common.exceptions.MooseDocsException("The 'data' setting is required.")
        data = eval(data)

        # Use Postprocessor file for data
        filename = settings['filename']
        if filename is not None:
            filename = common.check_filenames(filename)
            reader = mooseutils.PostprocessorReader(filename)

            for i, line in enumerate(data):
                data[i]['x'] = reader[line['x']].tolist()
                data[i]['y'] = reader[line['y']].tolist()

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True)
        scatter = ScatterToken(flt, data=data, layout=eval(settings['layout']))

        if flt is parent:
            scatter.attributes.update(**self.attributes(settings))

        return parent

class GraphHistogram(command.CommandComponent):
    """
    Histogram plot.
    """
    COMMAND = 'plot'
    SUBCOMMAND = 'histogram'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['data'] = (None, "Directly supply a list of dict items (i.e., JSON data) to the "
                                  "plotly plot command, see "
                                  "https://plot.ly/javascript/line-and-scatter"
                                  "for additional details.")
        settings['layout'] = (None, "Plotly layout settings for the chart, refer to "
                                        "https://plot.ly/javascript/reference/#layout "
                                        "for available options.")
        settings['filename'] = (None, "The name of a CSV file for extracting data, when used the "
                                "'x' and 'y' fields of the 'data' setting should be replaced by "
                                "column names or numbers.")
        settings['vectors'] = (None, "Name of postprocessor vector names to plot, default is all.")
        settings['names'] = (None, "Name to show on legend, by default the vector names are used.")
        settings['probability'] = (True, "True to plot with probability density normalization.")
        settings['bins'] = (0, "Number of bins to use, set to 0 for auto calculation.")
        settings['alpha'] = (1.0, "Set chart opacity alpha setting.")
        settings['title'] = ('', "Plot title")
        settings['xlabel'] = ('Value', "x-axis label")
        settings['ylabel'] = ('Probability', "y-axis label")
        settings['legend'] = ('legend', True, "True|False toggle for legend.")
        settings.update(floats.caption_settings())
        settings['prefix'] = ('Figure', settings['prefix'][1])
        return settings

    def createToken(self, parent, info, page, settings):

        # Build the JSON data for plotting
        data = settings['data']
        layout = settings['layout']

        # Use Postprocessor file for data
        filename = settings['filename']
        if filename is None:
            raise common.exceptions.MooseDocsException("The 'filename' setting is required.")
        filename = common.check_filenames(filename)
        reader = mooseutils.PostprocessorReader(filename)

        show_legend = True

        if data is None:
            vectors = reader.variables()
            if not settings['vectors'] is None:
                vectors = settings['vectors'].split(' ')

            names = []
            if settings['names'] is not None:
                names = settings['names'].split(' ')
                if not len(vectors) == len(names):
                    raise common.exceptions.MooseDocsException("Number of names must equal number of vectors.")
            else:
                names = vectors

            data = [dict() for i in range(len(vectors))];
            for i in range(len(vectors)):
                if not vectors[i] in reader.variables():
                    string = "The vector " + vectors[i] + " is not in " + filename
                    raise common.exceptions.MooseDocsException(string)
                data[i]['type'] = 'histogram'
                data[i]['x'] = reader[vectors[i]].tolist()
                data[i]['xbins'] = settings['bins']
                data[i]['opacity'] = settings['alpha']
                if settings['probability']:
                    data[i]['histnorm'] = 'probability'
                if len(names) > 0:
                    data[i]['name'] = names[i]

        else:
            data = eval(data)
            for i, line in enumerate(data):
                data[i]['x'] = reader[line['x']].tolist()
                data[i]['type'] = 'histogram'

        if layout is None:
            xaxis = dict()
            xaxis['title'] = settings['xlabel']

            yaxis = dict()
            yaxis['title'] = settings['ylabel']

            layout = dict()
            layout['xaxis'] = xaxis
            layout['yaxis'] = yaxis
            layout['showlegend'] = str(settings['legend'])
            layout['title'] = settings['title']

        else:
            layout = eval(layout)

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True)
        histogram = HistogramToken(flt, data=data, layout=layout)

        if flt is parent:
            histogram.attributes.update(**self.attributes(settings))

        return parent

class GraphTemplate(object):
    """
    Helper for loading plotly javascript templates.

    The template can contain markers such as MOOSEDOCS_ID, which will be
    replaced by the supplied "id" key when the __call__ method is used. Any trailing
    underscore from the key is used (e.g.,v"id_" == "id").
    """
    RE = re.compile(r'MOOSEDOCS_(?P<key>\w+)')
    def __init__(self, name):
        self.__template = common.read(os.path.join(os.path.dirname(__file__),
                                                   'templates',
                                                   name))

    def __call__(self, **kwargs):
        """Replace the markers with the supplied key, value pairs."""
        for key in list(kwargs.keys()):
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

    # Shares loaded template content across all instances
    HTML_TEMPLATE = GraphTemplate('scatter.js')

    def createHTML(self, parent, token, page):
        plot_id = str(uuid.uuid4())
        content = self.HTML_TEMPLATE(id_=plot_id,
                                     data=repr(token['data']),
                                     layout=repr(token['layout']))
        html.Tag(parent, 'div', id_=plot_id)
        html.Tag(parent, 'script', string=content)


    def createLatex(self, parent, token, page):

        args = []
        if self.extension['draft']:
            args.append('draft')
            loc = 'draft.pdf'
        else:
            layout = token['layout']
            layout.setdefault('font', dict(family='Computer Modern', size=12, color='#000000'))
            layout.setdefault('xaxis', dict())
            layout['xaxis'].setdefault('linewidth', 1)
            layout.setdefault('yaxis', dict())
            layout['yaxis'].setdefault('linewidth', 1)


            layout = plotly.graph_objs.Layout(**layout)
            fig = plotly.graph_objs.Figure(layout=layout)
            for data in token['data']:
                fig.add_scatter(**data)

            _, loc = tempfile.mkstemp(suffix='.pdf', dir=os.path.dirname(page.destination))
            plotly.io.write_image(fig, loc)

        args = [latex.create_settings(*args, width='\\textwidth')]
        latex.Command(parent, 'par', start='\n')
        latex.Command(parent, 'includegraphics', args=args, string=loc, start='\n', escape=False)

class RenderHistogram(components.RenderComponent):
    """Render a plotly histogram plot."""

    # Shares loaded template content across all instances
    HTML_TEMPLATE = GraphTemplate('scatter.js')

    def createHTML(self, parent, token, page):
        plot_id = str(uuid.uuid4())
        content = self.HTML_TEMPLATE(id_=plot_id,
                                     data=repr(token['data']),
                                     layout=repr(token['layout']))
        html.Tag(parent, 'div', id_=plot_id)
        html.Tag(parent, 'script', string=content)

    def createLatex(self, parent, token, page):

        args = []
        if self.extension['draft']:
            args.append('draft')
            loc = 'draft.pdf'
        else:
            layout = token['layout']
            layout.setdefault('font', dict(family='Computer Modern', size=12, color='#000000'))
            layout.setdefault('xaxis', dict())
            layout['xaxis'].setdefault('linewidth', 1)
            layout.setdefault('yaxis', dict())
            layout['yaxis'].setdefault('linewidth', 1)


            layout = plotly.graph_objs.Layout(**layout)
            fig = plotly.graph_objs.Figure(layout=layout)
            for data in token['data']:
                fig.add_trace(plotly.graph_objs.Histogram(**data))

            _, loc = tempfile.mkstemp(suffix='.pdf', dir=os.path.dirname(page.destination))
            plotly.io.write_image(fig, loc)

        args = [latex.create_settings(*args, width='\\textwidth')]
        latex.Command(parent, 'par', start='\n')
        latex.Command(parent, 'includegraphics', args=args, string=loc, start='\n', escape=False)

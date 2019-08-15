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
import tempfile

try:
    import plotly
    HAVE_PYTHON_PLOTLY = True
except ImportError:
    HAVE_PYTHON_PLOTLY = False

import mooseutils
from MooseDocs import common
from MooseDocs.extensions import command, floats
from MooseDocs.base import components, renderers
from MooseDocs.tree import tokens, html, latex

def make_extension(**kwargs):
    return GraphExtension(**kwargs)

ScatterToken = tokens.newToken('ScatterToken', data=[], layout=dict())

class GraphExtension(command.CommandExtension):
    """
    Extension for creating graphs via Plotly: https://plot.ly
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = (u'Figure', "The caption prefix (e.g., Fig.).")
        config['draft'] = (False, "Enable draft mode for LaTeX output.")
        return config

    def extend(self, reader, renderer):
        self.requires(command, floats)
        self.addCommand(reader, GraphScatter())
        renderer.add('ScatterToken', RenderScatter())

        if isinstance(renderer, renderers.HTMLRenderer):
            renderer.addJavaScript('plotly', "contrib/plotly/plotly.min.js", head=True)

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

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  bottom=True)
        scatter = ScatterToken(flt, data=data, layout=eval(self.settings['layout']))

        if flt is parent:
            scatter.attributes.update(**self.attributes)

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

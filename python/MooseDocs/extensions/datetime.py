#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import datetime
from ..base import components
from ..common import exceptions
from ..tree import tokens, html, latex
from . import command

def make_extension(**kwargs):
    return DateTimeExtension(**kwargs)

DateTime = tokens.newToken('DateTime', datetime=None, format=None, inline=True)

class DateTimeExtension(command.CommandExtension):
    """
    Adds ability to include date/time information.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, TodayCommand())
        renderer.add('DateTime', RenderDateTime())

class TodayCommand(command.CommandComponent):
    COMMAND = 'datetime'
    SUBCOMMAND = 'today'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['format'] = ('%Y-%m-%d', "The date format (see python datetime).")
        return settings

    def createToken(self, parent, info, page, settings):
        content = info['inline'] if 'inline' in info else info['block']
        if content:
            raise exceptions.MooseDocsException("Content is not supported for the 'datetime today' command.")

        DateTime(parent, datetime=datetime.date.today(),
                 inline='inline' in info,
                 format=settings['format'])
        return parent

class RenderDateTime(components.RenderComponent):

    def createHTML(self, parent, token, page):
        html.Tag(parent, 'span' if token['inline'] else 'p',
                 class_='moose-datetime',
                 string=token['datetime'].strftime(token['format']))
        return parent

    def createLatex(self, parent, token, page):
        latex.String(parent, content=token['datetime'].strftime(token['format']))
        return parent

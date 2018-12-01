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

from MooseDocs.base import components
from MooseDocs.extensions import command, floats
from MooseDocs.tree import tokens, html

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

Image = tokens.newToken('Image', src=u'')
Video = tokens.newToken('Video', src=u'', controls=True, autoplay=True, loop=True)

class MediaExtension(command.CommandExtension):
    """
    Extension for including images and movies, using the !media command.
    """

    @staticmethod
    def defaultConfig():
        config = components.Extension.defaultConfig()
        config['prefix'] = (u'Figure', "The caption prefix (e.g., Fig.).")
        return config

    def extend(self, reader, renderer):
        self.requires(command, floats)

        self.addCommand(reader, ImageCommand())
        self.addCommand(reader, VideoCommand())

        renderer.add('Image', RenderImage())
        renderer.add('Video', RenderVideo())

class MediaCommandBase(command.CommandComponent):
    """Base class for image and video tag creation."""
    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings.update(floats.caption_settings())
        return settings

    def createMedia(self, parent, info, page, src, **kwargs):
        pass # a required method for base classes

    def createToken(self, parent, info, page):

        # Determine the location of the media
        src = info['subcommand']
        if src.startswith('http'):
            location = src
        else:
            node = self.translator.findPage(src)
            location = unicode(node.relativeSource(page))

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  **self.attributes)
        self.createMedia(flt, info, page, location, **self.attributes)
        if flt.children[0].name == 'Caption':
            cap = flt.children[0]
            cap.parent = None
            cap.parent = flt
        return parent

class ImageCommand(MediaCommandBase):
    COMMAND = 'media'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg', None)

    def createMedia(self, parent, info, page, src, **kwargs):
        if parent.name == 'Float':
            return Image(parent, src=src)
        else:
            return Image(parent, src=src, **kwargs)

class VideoCommand(MediaCommandBase):
    COMMAND = 'media'
    SUBCOMMAND = ('ogg', 'webm', 'mp4')

    @staticmethod
    def defaultSettings():
        settings = MediaCommandBase.defaultSettings()
        settings['controls'] = (True, "Display the video player controls.")
        settings['loop'] = (False, "Automatically loop the video.")
        settings['autoplay'] = (False, "Automatically start playing the video.")
        return settings

    def createMedia(self, parent, info, page, src, **kwargs):
        return Video(parent,
                     src=src,
                     controls=self.settings['controls'],
                     loop=self.settings['loop'],
                     autoplay=self.settings['autoplay'],
                     **kwargs)

class RenderImage(components.RenderComponent):

    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        return html.Tag(parent, 'img', token, src=token['src'])

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        tag.addClass('materialboxed', 'moose-image')
        return tag

    def createLatex(self, parent, token, page):
        pass

class RenderVideo(components.RenderComponent):
    def createHTML(self, parent, token, page): #pylint: disable=no-self-use
        video = html.Tag(parent, 'video', token, src=token['src'])
        _, ext = os.path.splitext(token['src'])
        html.Tag(video, 'source', src=token['src'], type_="video/{}".format(ext[1:]))

        video['width'] = '100%'
        if token['controls']:
            video['controls'] = 'controls'
        if token['autoplay']:
            video['autoplay'] = 'autoplay'
        if token['loop']:
            video['loop'] = 'loop'

    def createLatex(self, parent, token, page):
        pass

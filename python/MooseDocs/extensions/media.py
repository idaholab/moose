#pylint: disable=missing-docstring
import os

import MooseDocs
from MooseDocs.common import exceptions
from MooseDocs.base import components
from MooseDocs.extensions import command, floats
from MooseDocs.tree import tokens, html
from MooseDocs.tree.base import Property

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

class Image(tokens.Token):
    PROPERTIES = [Property('src', required=True, ptype=unicode)]

class Video(tokens.Token):
    PROPERTIES = [Property('src', required=True, ptype=unicode),
                  Property('controls', default=True, ptype=bool),
                  Property('autoplay', default=True, ptype=bool),
                  Property('loop', default=True, ptype=bool)]

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

        self.addCommand(ImageCommand())
        self.addCommand(VideoCommand())

        renderer.add(Image, RenderImage())
        renderer.add(Video, RenderVideo())

class MediaCommandBase(command.CommandComponent):
    """Base class for image and video tag creation."""
    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['caption'] = (None, "The caption to use for the media content.")
        settings['prefix'] = (None, "The caption prefix.")
        return settings

    def createMedia(self, parent, src, **kwargs):
        pass # a required method for base classes

    def createToken(self, info, parent):

        # Determine the location of the media
        src = info['subcommand']
        if src.startswith('http') or self.translator.current is None:
            location = src
        else:
            node = self.translator.current.findall(src, exc=exceptions.TokenizeException)[0]
            location = unicode(node.relativeSource(self.translator.current))

        # Caption settings
        cap = self.settings['caption']
        key = self.settings['id']
        prefix = self.settings['prefix'] if self.settings['prefix'] \
                 is not None else self.extension['prefix']

        # Create float image
        if cap or key:
            flt = floats.Float(parent, img=True, **self.attributes)
            self.createMedia(flt, location)

            # Add caption
            if key:
                caption = floats.Caption(flt, key=key, prefix=prefix)
                if cap:
                    self.translator.reader.parse(caption, cap, MooseDocs.INLINE)
            elif cap:
                caption = floats.Caption(flt)
                self.translator.reader.parse(caption, cap, MooseDocs.INLINE)

        # Create regular image
        else:
            self.createMedia(parent, location, **self.attributes)

        return parent

class ImageCommand(MediaCommandBase):
    COMMAND = 'media'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg', None)

    def createMedia(self, parent, src, **kwargs):
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

    def createMedia(self, parent, src, **kwargs):
        return Video(parent,
                     src=src,
                     controls=self.settings['controls'],
                     loop=self.settings['loop'],
                     autoplay=self.settings['autoplay'],
                     **kwargs)

class RenderImage(components.RenderComponent):

    def createHTML(self, token, parent): #pylint: disable=no-self-use
        return html.Tag(parent, 'img', src=token.src, **token.attributes)

    def createMaterialize(self, token, parent):
        tag = self.createHTML(token, parent)
        tag.addClass('materialboxed', 'moose-image')
        return tag

    def createLatex(self, token, parent):
        pass

class RenderVideo(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        video = html.Tag(parent, 'video', **token.attributes)
        _, ext = os.path.splitext(token.src)
        html.Tag(video, 'source', src=token.src, type_="video/{}".format(ext[1:]))

        video['width'] = '100%'
        if token.controls:
            video['controls'] = 'controls'
        if token.autoplay:
            video['autoplay'] = 'autoplay'
        if token.loop:
            video['loop'] = 'loop'

    def createLatex(self, token, parent):
        pass

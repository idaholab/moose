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
import logging
from MooseDocs.common import exceptions
from MooseDocs.base import components, LatexRenderer
from MooseDocs.extensions import command, floats
from MooseDocs.tree import tokens, html, latex

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

Image = tokens.newToken('Image', src=u'', tex=u'')
Video = tokens.newToken('Video', src=u'', tex=u'',
                        controls=True, autoplay=True, loop=True)

class MediaExtensionBase(command.CommandExtension):

    def latexImage(self, parent, token, page, src):

        args = []
        style = latex.parse_style(token)
        width = style.get('width', None)
        if width:
            if width.endswith('%'):
                width = u'{}\\textwidth'.format(int(width[:-1])/100.)
            args.append(latex.Bracket(string=u'width={}'.format(width), escape=False))

        if style.get('text-align', None) == 'center':
            env = latex.Environment(parent, 'center')
        else:
            env = parent

        node = self.translator.findPage(src)
        fname = os.path.join(self.translator.destination, node.local)
        img = latex.Command(env, 'includegraphics', string=fname, args=args, escape=False)
        return img

class MediaExtension(MediaExtensionBase):
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

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('graphicx')
            renderer.addPackage('xcolor')

class ImageCommand(command.CommandComponent):
    COMMAND = 'media'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg', None)

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['latex_src'] = (None, "Image to utilize when rendering with LaTeX")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page):

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  bottom=True, **self.attributes)
        img = Image(flt, src=info['subcommand'], tex=self.settings['latex_src'])
        if flt is parent:
            img.attributes.update(**self.attributes)
        return parent

class VideoCommand(command.CommandComponent):
    COMMAND = 'media'
    SUBCOMMAND = ('ogv', 'webm', 'mp4')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['latex_src'] = (None, "Image to utilize when rendering with LaTeX")
        settings['controls'] = (True, "Display the video player controls.")
        settings['loop'] = (False, "Automatically loop the video.")
        settings['autoplay'] = (False, "Automatically start playing the video.")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page):

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  bottom=True, img=True)
        vid = Video(flt,
                    src=info['subcommand'],
                    tex=self.settings['latex_src'],
                    controls=self.settings['controls'],
                    loop=self.settings['loop'],
                    autoplay=self.settings['autoplay'])

        if flt is parent:
            vid.attributes.update(**self.attributes)

        return parent

class RenderImage(components.RenderComponent):

    def createHTML(self, parent, token, page):

        # Determine the location of the media
        src = token['src']
        if not src.startswith('http'):
            node = self.translator.findPage(src)
            src = unicode(node.relativeSource(page))

        return html.Tag(parent, 'img', token, src=src)

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        tag.addClass('materialboxed', 'moose-image')
        return tag

    def createLatex(self, parent, token, page):
        src = token['tex'] or token['src']

        _, ext = os.path.splitext(src)
        if src.startswith('http') and (ext not in ('.jpg', '.png', '.pdf')):
            msg = "Online images and images with the '{}' extension are not supported. The image " \
                  "should be downloaded and converted to a '.jpg', '.png', or '.pdf'. If the " \
                  "online version is desired for the website, the 'latex_src' setting can be used."
            raise exceptions.MooseDocsException(msg, ext)
        elif src.startswith('http'):
            msg = "Online images are not supported. The image should be downloaded. If the " \
                  "online version is desired for the website, the 'latex_src' setting can be used."
            raise exceptions.MooseDocsException(msg, ext)
        elif ext not in ('.jpg', '.png', '.pdf'):
            msg = "Images with the '{}' extension are not supported. The image " \
                  "should be converted to a '.jpg', '.png', or '.pdf'."
            raise exceptions.MooseDocsException(msg, ext)

        self.extension.latexImage(parent, token, page, src)
        return parent

class RenderVideo(components.RenderComponent):
    def createHTML(self, parent, token, page):

        node = None
        src = token['src']
        if not src.startswith('http'):
            node = self.translator.findPage(src)
            src = unicode(node.relativeSource(page))

        video = html.Tag(parent, 'video', token, src=src)
        _, ext = os.path.splitext(src)
        html.Tag(video, 'source', src=src, type_="video/{}".format(ext[1:]))

        video['width'] = '100%'
        if token['controls']:
            video['controls'] = 'controls'
        if token['autoplay']:
            video['autoplay'] = 'autoplay'
        if token['loop']:
            video['loop'] = 'loop'

    def createLatex(self, parent, token, page):

        src = token['tex']
        _, ext = os.path.splitext(src)
        if not src:
            msg = "Videos ({}) are not supported with LaTeX output, the 'latex_src' setting " \
                  "should be utilized to supply an image ('.jpg', '.png', or '.pdf')."
            raise exceptions.MooseDocsException(msg, token['src'], ext)
        elif ext not in ('.jpg', '.png', '.pdf'):
            msg = "Images ({}) with the '{}' extension are not supported. The image " \
                  "should be converted to a '.jpg', '.png', or '.pdf'."
            raise exceptions.MooseDocsException(msg, src, ext)

        img = self.extension.latexImage(parent, token, page, src)
        if token['src'].startswith('http'):
            latex.String(img.parent, content=u'\\newline(', escape=False)
            latex.Command(img.parent, 'url', string=token['src'])
            latex.String(img.parent, content=u')')

        return parent

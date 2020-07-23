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
from ..common import exceptions
from ..base import components, Extension, LatexRenderer
from ..tree import tokens, html, latex
from . import command, floats

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

Image = tokens.newToken('Image', src='', tex='')
Video = tokens.newToken('Video', src='', tex='',
                        controls=True, autoplay=True, loop=True, tstart=None, tstop=None)

class MediaExtensionBase(command.CommandExtension):

    def latexImage(self, parent, token, page, src):

        args = []
        style = latex.parse_style(token)
        width = style.get('width', None)
        if width:
            if width.endswith('%'):
                width = '{}\\textwidth'.format(int(width[:-1])/100.)
            args.append(latex.Bracket(string='width={}'.format(width), escape=False))

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
        config = MediaExtensionBase.defaultConfig()
        config['prefix'] = ('Figure', "The caption prefix (e.g., Fig.).")
        return config

    def initPage(self, page):
        page[self.name] = dict(prefix=self.get('prefix'))

    def preRead(self, page):
        page['prefix'] = page[self.name]['prefix']

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
    SUBCOMMAND = ('ogv', 'webm', 'mp4', 'm4v', 'youtube')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['youtube_embed_src'] = (None, "Youtube embed link, only recognized when the subcommand is 'youtube'.")
        settings['latex_src'] = (None, "Image to utilize when rendering with LaTeX")
        settings['controls'] = (True, "Display the video player controls (not compatible with YouTube).")
        settings['loop'] = (False, "Automatically loop the video (not compatible with YouTube).")
        settings['autoplay'] = (False, "Automatically start playing the video (not compatible with YouTube).")
        settings['tstart'] = (None, "Time (sec) to start video.")
        settings['tstop'] = (None, "Time (sec) to stop video.")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page):

        flt = floats.create_float(parent, self.extension, self.reader, page, self.settings,
                                  bottom=True, img=True)

        if info['subcommand'] == "youtube":
            vid = Video(flt,
                        src=self.settings['youtube_embed_src'],
                        youtube=True,
                        tex=self.settings['latex_src'],
                        tstart=self.settings['tstart'],
                        tstop=self.settings['tstop'])
        else:
            vid = Video(flt,
                        src=info['subcommand'],
                        youtube=False,
                        tex=self.settings['latex_src'],
                        controls=self.settings['controls'],
                        loop=self.settings['loop'],
                        autoplay=self.settings['autoplay'],
                        tstart=self.settings['tstart'],
                        tstop=self.settings['tstop'])

        if flt is parent:
            vid.attributes.update(**self.attributes)

        return parent

class RenderImage(components.RenderComponent):

    def createHTML(self, parent, token, page):

        # Determine the location of the media
        src = token['src']
        if not src.startswith('http'):
            node = self.translator.findPage(src)
            src = str(node.relativeSource(page))

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
            src = str(node.relativeSource(page))

        tstart = token['tstart']
        tstop = token['tstop']

        if token['youtube']:
            if tstart and tstop:
                src += '?start={:.0f}&end={:.0f};'.format(tstart, tstop)
            elif tstart:
                src += '?t={:.0f};'.format(tstart)
            elif tstop:
                src += '?start=0&end={:.0f};'.format(tstop)

            div = html.Tag(parent, 'div', style="text-align:center;")
            # using standard YouTube width and height for embedded videos as of July 2020
            video = html.Tag(div, 'iframe', token, width="560", height="315", src=src,
                             frameborder="0",
                             allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture",
                             allowfullscreen="allowfullscreen")
        else:
            if tstart and tstop:
                src += '#t={},{}'.format(tstart, tstop)
            elif tstart:
                src += '#t={}'.format(tstart)
            elif tstop:
                src += '#t=0,{}'.format(tstop)

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
            latex.String(img.parent, content='\\newline(', escape=False)
            latex.Command(img.parent, 'url', string=token['src'])
            latex.String(img.parent, content=')')

        return parent

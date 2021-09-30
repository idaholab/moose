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
import mooseutils
from ..common import exceptions
from ..base import components, Extension, LatexRenderer
from ..tree import tokens, html, latex
from . import command, floats

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

Image = tokens.newToken('Image', src='', tex='', dark='')
Video = tokens.newToken('Video', src='', tex='', youtube=False,
                        controls=True, poster=None, autoplay=True, loop=True, tstart=None, tstop=None)

class MediaExtension(command.CommandExtension):
    """
    Extension for including images and movies, using the !media command.
    """

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['prefix'] = ('Figure', "The caption prefix (e.g., Fig.).")
        return config

    def initPage(self, page):
        page[self.name] = dict()

    def preRead(self, page):
        page['prefix'] = page[self.name].get('prefix', self.get('prefix'))

    def extend(self, reader, renderer):
        self.requires(command, floats)

        self.addCommand(reader, ImageCommand())
        self.addCommand(reader, VideoCommand())

        renderer.add('Image', RenderImage())
        renderer.add('Video', RenderVideo())

        if isinstance(renderer, LatexRenderer):
            renderer.addPackage('graphicx')
            renderer.addPackage('xcolor')

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

class ImageCommand(command.CommandComponent):
    COMMAND = 'media'
    SUBCOMMAND = ('jpg', 'jpeg', 'gif', 'png', 'svg')

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['latex_src'] = (None, "Image to utilize when rendering with LaTeX")
        settings['dark_src'] = (None, "Image to utilize with dark HTML theme")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page, settings):

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True, **self.attributes(settings))
        img = Image(flt, src=info['subcommand'], dark=settings['dark_src'],
                    tex=settings['latex_src'])
        if flt is parent:
            img.attributes.update(**self.attributes(settings))
        return parent

class VideoCommand(command.CommandComponent):
    COMMAND = 'media'
    SUBCOMMAND = ('ogv', 'webm', 'mp4', 'm4v', None)

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['latex_src'] = (None, "Image to utilize when rendering with LaTeX")
        settings['controls'] = (True, "Display the video player controls (not compatible with YouTube).")
        settings['loop'] = (False, "Automatically loop the video (not compatible with YouTube).")
        settings['autoplay'] = (False, "Automatically start playing the video (not compatible with YouTube).")
        settings['tstart'] = (None, "Time (sec) to start video.")
        settings['tstop'] = (None, "Time (sec) to stop video.")
        settings['poster'] = (None, "Add a 'poster' image the the video")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page, settings):
        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True, img=True, **self.attributes(settings))

        vid = Video(flt,
                    src=info['subcommand'],
                    youtube='www.youtube.com' in info['subcommand'],
                    tex=settings['latex_src'],
                    controls=settings['controls'],
                    poster=settings['poster'],
                    loop=settings['loop'],
                    autoplay=settings['autoplay'],
                    tstart=settings['tstart'],
                    tstop=settings['tstop'])

        if flt is parent:
            vid.attributes.update(**self.attributes(settings))

        return parent

class RenderImage(components.RenderComponent):

    def createHTML(self, parent, token, page):

        # Determine the location of the media
        src = token['src']
        if not src.startswith('http'):
            node = self.translator.findPage(src)
            src = str(node.relativeSource(page))

        pic = html.Tag(parent, 'picture')
        if token['dark']:
            html.Tag(pic, 'source', srcset=token['dark'], media='(prefers-color-scheme: dark)')
        html.Tag(pic, 'img', token, src=src)
        return pic

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

        if token['youtube']:
            src = token['src']
            tstart = token['tstart']
            tstop = token['tstop']
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
            video = self.addVideoHelper(parent, token, page)

    def addVideoHelper(self, parent, token, page):
        src = token['src']
        if not src.startswith('http'):
            node = self.translator.findPage(src)
            src = str(node.relativeSource(page))

        tstart = token['tstart']
        tstop = token['tstop']

        if tstart and tstop:
            src += '#t={},{}'.format(tstart, tstop)
        elif tstart:
            src += '#t={}'.format(tstart)
        elif tstop:
            src += '#t=0,{}'.format(tstop)

        # Need to place HTML video elements in their own div element so that the controls render
        # properly (they can overlap with the video container and cause weird looking artifacts).
        div = html.Tag(parent, 'div', token, class_='moose-video-div')
        video = html.Tag(div, 'video', class_='moose-video')
        _, ext = os.path.splitext(src)
        source = html.Tag(video, 'source', src=src)

        source["type"] = "video/{}".format(ext[1:])

        # Set attributes for HTML video element
        video['width'] = '100%'
        if token['poster'] is not None:
            video['poster'] = "/" + self.translator.findPage(token['poster']).local

        # Ensure that bool flags are boolean
        for key in ['controls', 'loop', 'autoplay']:
            value = token[key]
            if isinstance(value, str):
                token[key] = mooseutils.str2bool(value)

        video['loop'] = token['loop']
        video['autoplay'] = token['autoplay']
        video['controls'] = token['controls']

        #https://developer.mozilla.org/en-US/docs/Web/HTML/Element/video
        #In some browsers (e.g. Chrome 70.0) autoplay doesn't work if no muted attribute is present."
        if video['autoplay']:
            video['muted'] = True

        return video


    def createLatex(self, parent, token, page):

        src = token['tex']
        if not src:
            msg = "Videos ({}) are not supported with LaTeX output, the 'latex_src' setting " \
                  "should be utilized to supply an image ('.jpg', '.png', or '.pdf')."
            raise exceptions.MooseDocsException(msg, token['src'])

        _, ext = os.path.splitext(src)
        if ext not in ('.jpg', '.png', '.pdf'):
            msg = "Videos ({}) with the '{}' extension are not supported. The image " \
                  "should be converted to a '.jpg', '.png', or '.pdf'."
            raise exceptions.MooseDocsException(msg, src, ext)

        img = self.extension.latexImage(parent, token, page, src)
        if token['src'].startswith('http'):
            latex.String(img.parent, content='\\newline(', escape=False)
            latex.Command(img.parent, 'url', string=token['src'])
            latex.String(img.parent, content=')')

        return parent

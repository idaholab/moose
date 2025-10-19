#* This file is part of the MOOSE framework
#* https://mooseframework.inl.gov
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import subprocess
import logging
import mooseutils
from ..common import exceptions, report_error
from ..base import components, Extension, LatexRenderer
from ..tree import tokens, html, latex, pages
from . import command, floats

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return MediaExtension(**kwargs)

Image = tokens.newToken('Image', src='', tex='', dark='', href='', alt='')
Video = tokens.newToken('Video', src='', tex='', quicktime='', youtube=False, dark='',
                        controls=True, poster=None, autoplay=True, loop=True,
                        tstart=None, tstop=None, alt='')

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
        page['script_files'] = set() # storage for image file created by scripts, see ScriptCommand object

    def preRender(self, page, ast, result):

        # add a page to the local process translator for images created by scripts
        for extra in page['script_files']:
            plot_page = pages.File(extra[0], source=extra[1], base=self.translator.destination)
            if self.translator.findPage(plot_page.local, throw_on_zero=False) is None:
                self.translator.addPage(plot_page)

    def postWrite(self, page):

        # write (i.e., copy ) the images created by scripts on the current process
        for extra in page['script_files']:
            plot_page = pages.File(extra[0], source=extra[1], base=self.translator.destination)
            self.translator.renderer.write(plot_page)

    def extend(self, reader, renderer):
        self.requires(command, floats)

        self.addCommand(reader, ScriptCommand())
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
        settings['link'] = (None, "Anchor URL to navigate to upon being clicked")
        settings['alt'] = (None, "Alt text describing image (defaults to caption)")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page, settings):

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True, **self.attributes(settings))
        img = Image(flt, src=info['subcommand'], dark=settings['dark_src'],
                    tex=settings['latex_src'], href=settings['link'],
                    alt=settings['alt'] or settings['caption'])
        if not (settings['alt'] or settings['caption']):
            msg = "Image has no 'caption' or 'alt' text. Provide 'alt' text to improve accessibility of website."
            LOG.warning(report_error(msg, page.source, info.line, info[0], prefix='WARNING'))

        if flt is parent:
            img.attributes.update(**self.attributes(settings))
        return parent

class ScriptCommand(ImageCommand):
    COMMAND = 'media'
    SUBCOMMAND = ('py')

    @staticmethod
    def defaultSettings():
        settings = ImageCommand.defaultSettings()
        settings['image_name'] = (None, "Name of image created by the Python plot script, defaults to the name of the script with .png extension")
        settings['alt'] = (None, "Alt text describing image (defaults to caption)")
        settings.update(floats.caption_settings())
        return settings

    def createToken(self, parent, info, page, settings):
        # Find the plot script
        script_page = self.translator.findPage(info['subcommand'])
        script_path = script_page.source
        script_localname = script_page.local
        script_absdir, script_name = os.path.split(script_path)
        script_localdir, script_name = os.path.split(script_localname)

        # Append MOOSE python to the PYTHONPATH when we run the script
        # so that our utilities can be used without extra path appends
        this_dir = os.path.dirname(os.path.abspath(__file__))
        python_dir = os.path.abspath(os.path.join(this_dir, '..', '..'))
        run_env = os.environ.copy()
        run_env['PYTHONPATH'] = f'{python_dir}:' + os.environ.get('PYTHONPATH', '')

        # Generate the plot
        LOG.info("Executing plot script %s", script_path)
        result = subprocess.run(["python", script_path], capture_output=True, text=True, env=run_env)
        if result.returncode != 0:
            msg = "Failed to execute python script '{}':\n{}"
            raise exceptions.MooseDocsException(msg, script_path, result.stderr)

        # Currently the plot is assumed to reside in the same directory as the plot script.
        plot_name = settings['image_name'] or os.path.basename(script_path).replace('.py', '.png')
        plot_path = os.path.join(script_absdir, plot_name)
        plot_localname = os.path.join(script_localdir, plot_name)

        # Throw error if the expected plot does not exist
        if not os.path.isfile(plot_path):
            LOG.error("The plot script '%s' must generate the plot '%s'", script_localname, plot_localname)

        # Add plot page information to the global page data for processing by the extension, see MediaExtension object
        page['script_files'].add((plot_localname, plot_path))

        flt = floats.create_float(parent, self.extension, self.reader, page, settings,
                                  bottom=True, **self.attributes(settings))
        img = Image(flt, src=plot_name, dark=settings['dark_src'],
                    tex=settings['latex_src'],
                    alt=settings['alt'] or settings['caption'])
        if not (settings['alt'] or settings['caption']):
            msg = "Image has no 'caption' or 'alt' text. Provide 'alt' text to improve accessibility of website."
            LOG.warning(report_error(msg, page.source, info.line, info[0], prefix='WARNING'))
        if flt is parent:
            img.attributes.update(**self.attributes(settings))
        return parent

class VideoCommand(command.CommandComponent):
    COMMAND = 'media'
    SUBCOMMAND = ('ogv', 'webm', 'mp4', 'm4v', 'quicktime', None)

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
        settings['quicktime'] = (None, "Video to utilize Macintosh codecs (for alpha transparencies)")
        settings['dark_src'] = (None, "Image to utilize with dark HTML theme")
        settings['alt'] = (None, "Alt text describing image (defaults to caption)")
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
                    tstop=settings['tstop'],
                    dark=settings['dark_src'],
                    quicktime=settings['quicktime'],
                    alt=settings['alt'] or settings['caption'])
        if not (settings['alt'] or settings['caption']):
            msg = "Video has no 'caption' or 'alt' text. Provide 'alt' text to improve accessibility of website."
            LOG.warning(report_error(msg, page.source, info.line, info[0], prefix='WARNING'))

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

        if token['href']:
            # Remove any styles being set for the img tag so it does not pollute the anchor tag
            pic_link = html.Tag(parent, 'a', token, href=token["href"], style=None)
            pic = html.Tag(pic_link, 'picture')
        else:
            pic = html.Tag(parent, 'picture')
        if token['dark']:
            html.Tag(pic, 'source', srcset=token['dark'], media='(prefers-color-scheme: dark)')
        if token['alt']:
            html.Tag(pic, 'img', token, src=src, alt=token['alt'])
        else:
            html.Tag(pic, 'img', token, src=src)
        return pic

    def createMaterialize(self, parent, token, page):
        tag = self.createHTML(parent, token, page)
        if not token['href']:
            tag.addClass('materialboxed moose-image')
            # Add these to make materialboxed work better with
            # accessibility tools. Not ideal to force these onto a
            # <picture> element rather than use something clickable
            # like <a>, but Materialize doesn't seem to want to work
            # correctly with anything else.
            tag['role'] = 'button'
            tag['tabindex'] = '0'
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

        if token['dark']:
            html.Tag(video, 'source', src=token['dark'], media='(prefers-color-scheme: dark)')

        if token['quicktime']:
            html.Tag(video, 'source', src=token['quicktime'], type='video/quicktime')

        source = html.Tag(video, 'source', src=src)

        source["type"] = f"video/{ext[1:]}"

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

        if token['alt']:
            video['aria-label'] = token['alt']
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

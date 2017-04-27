import os
import math
import multiprocessing
import markdown
from distutils.dir_util import copy_tree
import logging
log = logging.getLogger(__name__)

import MooseDocs
from MooseDocsNode import MooseDocsNode
from MooseDocsMarkdownNode import MooseDocsMarkdownNode


def build_options(parser, site_dir=True):
    """
    Command-line options for build command.
    """
    parser.add_argument('--config-file', type=str, default='moosedocs.yml', help="The configuration file to use for building the documentation using MOOSE. (Default: %(default)s)")
    parser.add_argument('--num-threads', '-j', type=int, default=multiprocessing.cpu_count(), help="Specify the number of threads to build pages with.")
    parser.add_argument('--template', type=str, default='website.html', help="The template html file to utilize (Default: %(default)s).")
    if site_dir:
        parser.add_argument('--site-dir', type=str, default=os.path.join(MooseDocs.ROOT_DIR, 'site'), help="The location to build the website content (Default: %(default)s).")

def make_tree(directory, node, **kwargs):
    """
    Create the tree structure of NavigationNode/MooseDocsMarkdownNode objects
    """
    for p in os.listdir(directory):

        path = os.path.join(directory, p)
        if p in ['index.md', 'index.html']:
            continue

        if os.path.isfile(path) and (path.endswith('.md')):
            name = os.path.basename(path)[:-3]
            child = MooseDocsMarkdownNode(name=name, parent=node, md_file=path, **kwargs)

        elif os.path.isdir(path) and (p not in ['.', '..']):
            name = os.path.basename(path)
            md = os.path.join(path, 'index.md')
            if os.path.exists(md):
                child = MooseDocsMarkdownNode(name=name, parent=node, md_file=md, **kwargs)
            else:
                child = MooseDocsNode(name=name, parent=node, **kwargs)
            make_tree(path, child, **kwargs)

def flat(node):
    """
    Create a flat list of pages for parsing and generation.

    Args:
      node[NavigationNode]: The root node to flatten from
    """
    for child in node:
        if isinstance(child, MooseDocsMarkdownNode):
            yield child
        for c in flat(child):
            yield c


class Builder(object):
    """
    Object for building
    """
    def __init__(self, parser, site_dir, template, template_args, navigation):

        self._site_dir = site_dir

        content_dir = os.path.join(os.getcwd(), 'content')
        kwargs = {'parser': parser,
                  'site_dir': self._site_dir,
                  'navigation': MooseDocs.yaml_load(navigation),
                  'template': template,
                  'template_args': template_args}
        self._root = MooseDocsMarkdownNode(name='', md_file=os.path.join(content_dir, 'index.md'), **kwargs)
        make_tree(content_dir, self._root, **kwargs)
        self._pages = [self._root] + list(flat(self._root))

    def __iter__(self):
        """
        Allow direct iteration over pages contained in this object.
        """
        return self._pages.__iter__()

    def build(self, num_threads=multiprocessing.cpu_count()):
        """
        Build all the pages in parallel.
        """

        def make_chunks(l, n):
            n = int(math.ceil(len(l)/float(n)))
            for i in range(0, len(l), n):
                yield l[i:i + n]

        def build_pages(pages, lock):
            for page in pages:
                page.build(lock)

        jobs = []
        lock = multiprocessing.Lock()
        for chunk in make_chunks(self._pages, num_threads):
            p = multiprocessing.Process(target=build_pages, args=(chunk, lock))
            p.start()
            jobs.append(p)

        for job in jobs:
            job.join()

        self.copyFiles()

    def copyFiles(self):
        """
        Copy the css/js/fonts/media files for this project.
        """

        def helper(src, dst):
            if not os.path.exists(dst):
                os.makedirs(dst)
            if os.path.exists(src):
                copy_tree(src, dst)

        # Copy js/css/media from MOOSE and current projects
        for from_dir in [os.path.join(MooseDocs.MOOSE_DIR, 'docs'), os.getcwd()]:
            helper(os.path.join(from_dir, 'js'), os.path.join(self._site_dir, 'js'))
            helper(os.path.join(from_dir, 'css'), os.path.join(self._site_dir, 'css'))
            helper(os.path.join(from_dir, 'media'), os.path.join(self._site_dir, 'media'))

def build_site(config_file=None, site_dir=None, num_threads=None, template=None):
    """
    The main build command.
    """

    # Load the YAML configuration file
    config = MooseDocs.load_config(config_file, template=template)

    # Create the markdown parser
    extensions, extension_configs = MooseDocs.get_markdown_extensions(config)
    parser = markdown.Markdown(extensions=extensions, extension_configs=extension_configs)

    # Create object for storing pages to be generated
    builder = Builder(parser, site_dir, config['template'], config['template_arguments'], config['navigation'])

    # Create the html
    builder.build(num_threads=num_threads)
    return config, parser, builder

def build(*args, **kwargs):
    """
    The main build command.
    """
    build_site(*args, **kwargs)

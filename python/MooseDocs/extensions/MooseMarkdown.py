import os
import subprocess
import markdown

from MooseSourceFile import MooseSourceFile
from MooseInputBlock import MooseInputBlock
from MooseCppMethod import MooseCppMethod
from MooseSlideTreeprocessor import MooseSlideTreeprocessor

import MooseDocs
import utils


class MooseMarkdown(markdown.Extension):

    def __init__(self, *args, **kwargs):

        root = os.path.dirname(subprocess.check_output(['git', 'rev-parse', '--git-dir'], stderr=subprocess.STDOUT))

        self.config = dict()
        self.config['root'] = [root, "The root directory of the repository, if not provided the root is found using git."]
        self.config['make'] = [root, "The location of the Makefile responsible for building the application."]
        self.config['repo'] = ['', "The remote repository to create hyperlinks."]

        super(MooseMarkdown, self).__init__(*args, **kwargs)

    def extendMarkdown(self, md, md_globals):

        # Strip description from config
        config = dict()
        for key, value in self.config.iteritems():
            config[key] = value[0]

        #md.treeprocessors.add('moose_slides', MooseSlideTreeprocessor(md), '_end')
        md.inlinePatterns.add('moose_input_block', MooseInputBlock(config), '<image_link')
        md.inlinePatterns.add('moose_cpp_method', MooseCppMethod(config), '<image_link')
        md.inlinePatterns.add('moose_source', MooseSourceFile(config), '<image_link')

def makeExtension(*args, **kwargs):
    return MooseMarkdown(*args, **kwargs)

if __name__ == '__main__':

    md = markdown.Markdown(extensions=[makeExtension(repo='https://github.com/idaholab/moose/blob/master', make='/Users/slauae/projects/moose-doc/modules/')])
    md.convertFile(output='test.html',
                   input='/Users/slauae/projects/moose-doc/docs/documentation/generation/Overview.md')

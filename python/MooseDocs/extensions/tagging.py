#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os, re
import logging
from ..tree import tokens
from ..common import __init__
from . import command

"""
This extension defines the tagging command: !tag name=id pairs=key:value. The tag command will accept
a 'name' parameter string that represents the originating markdown file and is associated with an
arbitrary list of key:value pairs taken in by the 'pairs' parameter. The tagging extension checks
that all linked moose pages are unique and will not allow duplicate names in the dictionary.
Duplicate key value pairs are allowed, but a warning is issued and only one of the pairs is taken in.

ALERT: The tagging extension is experimental! If documentation tagging features are desired, please
request assistance from the MOOSE Framework development team.

Example Tag command in *.md:
!tag name=geochem pairs=keyg:valg keychem:valuechem

Example output tag dictionary for multiple pages, names, and key:value pairs:
{data:
[{name: "heatconduction", path: "moose/modules/heat_conduction/doc/content/modules/heat_conduction/index.md", key_vals: {keyheat: "valheat", key: "val", key1: "val1"}},
{name: "index", path: "moose/modules/doc/content/index.md", key_vals: {key1: "val1", keya: "val"}},
{name: "index2", path: "moose/modules/doc/content/index2.md", key_vals: {key1: "val1", keya: "val", thing1: "thing2"}},
{name: "geochem", path: "moose/modules/geochemistry/doc/content/modules/geochemistry/index.md", key_vals: {keyg: "valg", keychem: "valuechem"}},
{name: "vortex", path: "moose/modules/level_set/doc/content/modules/level_set/example_vortex.md", key_vals: {keyvor: "valvor", key: "val", key1: "val1"}}]
}
"""

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return TaggingExtension(**kwargs)

Tag = tokens.newToken('Tag', attr_name='', path='', key_vals=[])

class TaggingExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['allowed_keys'] = ([],
                                  "List of tag keys allowed in documentation pages. If empty, all " \
                                  "keys allowed.")
        config['js_file'] = (None,
                             "Javascript file used for filtering / search page.")
        # Disable by default
        config['active'] = (False, config['active'][1])
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, TaggingCommand())

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self._allowed_keys = self['allowed_keys']

        if self['js_file'] is None:
            msg = "No javascript file identified. The tagging extension will be disabled."
            LOG.warning(msg)
            self.setActive(False)

        if self['active'] is True:
            LOG.warning("The tagging extension is experimental! If documentation tagging features " \
                        "are desired, please request assistance from the MOOSE Framework development " \
                        "team.")

    def get_tag_data(self, *args):
        return self.getAttribute(*args)

    def set_tag_data(self, *args):
        return self.setAttribute(*args)

    @property
    def allowed_keys(self):
        return self._allowed_keys

    def postExecute(self):
        """
        At completion execution process, collect and process all current data attributes. Then,
        collect javascript template content, find/replace, and write to destination.
        """
        replace_str = ""
        for iter in self.getAttributeItems():
            if bool(re.search('tag_', iter[0])):
                tag_dict_str=str(iter[1])
                key_list_regex=self._allowed_keys+['name', 'path', 'key_vals']
                for entry in key_list_regex:
                    regex_replace=f"'{entry}':"
                    tag_dict_str=re.sub(regex_replace,entry+':', tag_dict_str)
                tag_dict_str=re.sub("'",'"', tag_dict_str)
                # Downstream js cannot handle double quotes
                tag_dict_str=re.sub("\"\"",'"', tag_dict_str)
                if len(replace_str) == 0:
                    replace_str += tag_dict_str
                else:
                    replace_str += "," + tag_dict_str

        replace_str = "{data:[" + replace_str + "]}"

        # Find the javascript file with error checking
        js_page = self.translator.findPage(self['js_file'])
        js_path = js_page.source

        # Setup path to write content
        js_dest_partpath = "js/" + os.path.basename(js_path)
        js_dest_path = os.path.join(self.translator.destination, js_dest_partpath)

        # Find and replace 'data:' dict structure within supplied javascript template file content,
        # save to javascript file in destination.
        with open(js_path,'r') as f:
            content=f.readlines()
            f.close()
        content[-1]=re.sub('\{data:\[\{.+\}\}\]\}',str(replace_str), content[-1])
        with open(js_dest_path, 'w') as f:
            f.writelines(content)
            f.close()


class TaggingCommand(command.CommandComponent):
    COMMAND= 'tag'
    SUBCOMMAND= None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['name'] = (None, 'ID name for page and associated key:value category:label pairs.')
        settings['pairs'] = (None, 'Key:value pairs representing categories and page-specific labels for each category.')
        return settings

    def createToken(self, parent, info, page, settings):
        """
        Process name and key:value pairs provided in the documentation file, check against
        previously-processed names and keys, and add new data to global attributes.
        """
        if settings['name'] is None:
            msg = "%s: No 'name' provided for page and associated tags; check markdown file. " \
                  "This page will not be added to the tag database!"
            LOG.error(msg, page.name)
            name = ''
        else:
            name=settings['name']
        if settings['pairs'] is None:
            msg = "%s: No key:value pairs provided; check markdown file and add desired pairs."
            LOG.error(msg, page.name)
            keylist=''
        else:
            keylist=settings['pairs'].split()
        mpath=re.sub(r'^.*?moose/', 'moose/', page.source)
        entry_key_values=[]
        for keys in keylist:
            key_vals=keys.split(':')
            entry_key_values.append([key_vals[0],key_vals[1]])

        good_keys=[]
        for pair in entry_key_values:
            if pair[0] not in self.extension.allowed_keys and len(self.extension.allowed_keys) > 0:
                msg = "%s: Provided 'key' not in allowed_keys (see config.yml); not adding the " \
                       "following to the database: %s"
                LOG.warning(msg, page.name, pair[0])
            elif len(good_keys) > 0 and pair[0] in [item[0] for item in good_keys]:
                msg = "%s: Following 'key' provided more than once; check markdown file: %s"
                LOG.error(msg, page.name, pair[0])
            else:
                good_keys.append([pair[0], pair[1]])

        if len(name) != 0: # Only add to tag database if 'name' is provided
            page_data = {'name':name, "path":mpath, "key_vals":dict(good_keys)}

            tag_id_name = ''
            if self.extension.get_tag_data("tag_" + name):
                msg = "%s: Tag page identifier already exists; not adding the following 'name' to " \
                    "dictionary: %s"
                LOG.warning(msg, page.name, name)
            else:
                tag_id_name = "tag_" + name
                self.extension.set_tag_data(tag_id_name, page_data)

            Tag(parent, attr_name=tag_id_name, path=mpath, key_vals=dict(good_keys))

        return parent

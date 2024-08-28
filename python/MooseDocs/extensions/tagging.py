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
[{name: "heatconduction", path: "/modules/heat_conduction/doc/content/modules/heat_conduction/index.md", link: "/heat_conduction/doc/content/modules/heat_conduction/index.html", key_vals: {keyheat: "valheat", key: "val", key1: "val1"}},
 {name: "index", path: "moose/modules/doc/content/index.md", link: "/doc/content/index.html", key_vals: {key1: "val1", keya: "val"}},
 {name: "index2", path: "moose/modules/doc/content/index2.md", link: "/doc/content/index2.html", key_vals: {key1: "val1", keya: "val", thing1: "thing2"}},
 {name: "geochem", path: "moose/modules/geochemistry/doc/content/modules/geochemistry/index.md", link: "/geochemistry/doc/content/modules/geochemistry/index.html", key_vals: {keyg: "valg", keychem: "valuechem"}},
 {name: "vortex", path: "moose/modules/level_set/doc/content/modules/level_set/example_vortex.md", link: "/level_set/doc/content/modules/level_set/example_vortex.html", key_vals: {keyvor: "valvor", key: "val", key1: "val1"}}]
}
"""

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return TaggingExtension(**kwargs)

Tag = tokens.newToken('Tag', attr_name='', path='', description='', image='', key_vals=[])

class TaggingExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['allowed_keys'] = ([],
                                  "List of tag keys allowed in documentation pages. If empty, all " \
                                  "keys allowed.")
        config['js_file'] = (None,
                             "Javascript file used for filtering / search page.")
        config['csv_file'] = (None, "CSV file used for examining the tag database")
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
        # Helper for CSV data output
        name_key_val_dict = {}
        create_csv = (self['csv_file'] is not None)

        # Data dictionary saved as a string
        replace_str = ""
        for iter in self.getAttributeItems():
            print(iter)
            if bool(re.search('tag_', iter[0])):
                tag_dict_str=str(iter[1])
                key_list_regex = ['name', 'path', 'image', 'description', 'key_vals'] + self._allowed_keys
                for entry in key_list_regex:

                    # Remove single quotes around entry
                    regex_replace = f"'{entry}':"
                    tag_dict_str = re.sub(regex_replace, entry+':', tag_dict_str)

                    # add relative link built from the path at the end of the tagging entry
                    if (entry == 'path'):
                        path_value = tag_dict_str.split("path: ")[1].split(',')[0].replace("'", "")

                        # splitting the path is dangerous, even at content/, as more than one tagged page could have
                        # the same name.
                        if 'content/' in path_value:
                            path_value_cut = path_value.split('content/')[1]
                        else:
                            path_value_cut = path_value.split('/')[-1]

                        # Find the relative path from the filter page (assumed at filter/index.html)
                        # Check that there is no ambiguity
                        target_page = self.translator.findPages(path_value_cut)
                        filter_page = self.translator.findPages("filter/index.html")
                        if len(target_page) != 1:
                            LOG.error(str(len(target_page)) + " pages found after truncating address when "
                                      "tagging for page initially at address: " + path_value)
                        if len(filter_page) != 1:
                            LOG.warning(str(len(filter_page)) + " pages have been found for the filter page "
                                        "when building relative links for tagged pages!")
                        # We did not find the pages, thus cannot search for their relative path
                        # So we simply take the full path value and use it to create the link
                        if (len(target_page) == 0 or len(filter_page) == 0):
                            link_value = '/' + path_value_cut.replace('.md', '.html')
                        else:
                            target_page = target_page[0]
                            filter_page = filter_page[0]
                            link_value = target_page.relativeDestination(filter_page)

                        # Insert the link into the dictionary string
                        index = tag_dict_str.find(', key_vals')
                        tag_dict_str = tag_dict_str[:index] + ', link: "' + link_value + '"' + tag_dict_str[index:]

                tag_dict_str=re.sub("'",'"', tag_dict_str)
                # Downstream js cannot handle double quotes
                tag_dict_str=re.sub("\"\"",'"', tag_dict_str)
                if len(replace_str) == 0:
                    replace_str += tag_dict_str
                else:
                    replace_str += "," + tag_dict_str

            # Save some data for the CSV output
            if (create_csv):
                name_key_val_dict[iter[1]['name']] = iter[1]['key_vals']

        # Replace the dummy tag dictionary in the JS file with the collected dictionary from parsing the pages
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

        # Write the CSV file if requested
        if (create_csv):
            csv_path = self['csv_file']
            entry_names = name_key_val_dict.keys()
            # Get all the unique keys, sort them
            all_keys = []
            for each_dict in  name_key_val_dict.values():
                all_keys += each_dict.keys()
            all_keys = list(set(all_keys))
            all_keys.sort()

            # Fill in missing keys for tags that dont have them all
            for entry in entry_names:
                for key in all_keys:
                    name_key_val_dict[entry].setdefault(key, ' - ')

            with open(csv_path,'w') as f:
                # Write out 'names' (the word) then keys as a header
                f.write('names,' + ",".join(all_keys) + "\n")
                for entry in entry_names:
                    # Write the name (remove commas) and all all the values
                    entry_dict = dict(sorted(name_key_val_dict[entry].items()))
                    f.write(entry.replace(",", "") + ',' + ",".join(entry_dict.values()) + "\n")
                f.close()


class TaggingCommand(command.CommandComponent):
    COMMAND= 'tag'
    SUBCOMMAND= None

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        settings['name'] = (None, 'ID name for page and associated key:value category:label pairs.')
        settings['pairs'] = (None, 'Key:value pairs representing categories and page-specific labels for each category.')
        settings['image'] = (None, 'Link to an image to display for this entry')
        settings['description'] = (None, 'Description of the entry')
        return settings

    def createToken(self, parent, info, page, settings):
        """
        Process name and key:value pairs provided in the documentation file, check against
        previously-processed names and keys, and add new data to global attributes.
        """
        # Process input data
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
        # Downstream javascript does not support empty fields
        if settings['description'] is None:
            description = settings['name']
        else:
            description = settings['description']
        if settings['image'] is None:
            image = 'No Image'
        else:
            image=settings['image']
        mpath=re.sub(r'^.*?moose/', 'moose/', page.source)
        entry_key_values=[]
        for keys in keylist:
            key_vals=keys.split(':')
            entry_key_values.append([key_vals[0],key_vals[1]])

        # Check keys
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

        # Form tag token
        if len(name) != 0: # Only add to tag database if 'name' is provided
            page_data = {'name':name, "path":mpath, "description":description, "image":image, "key_vals":dict(good_keys)}

            tag_id_name = ''
            if self.extension.get_tag_data("tag_" + name):
                msg = "%s: Tag page identifier already exists; not adding the following 'name' to " \
                    "dictionary: %s"
                LOG.warning(msg, page.name, name)
            else:
                tag_id_name = "tag_" + name
                self.extension.set_tag_data(tag_id_name, page_data)

            Tag(parent, attr_name=tag_id_name, path=mpath, description=description, image=image, key_vals=dict(good_keys))

        return parent

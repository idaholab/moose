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
from . import tagging

"""
This extension performs automatic tagging of the source documentation.
- only pages using the "!syntax parameters" command are included
See tagging.py for more explanation about the output tag dictionary
"""

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return SourceTaggingExtension(**kwargs)

class SourceTaggingExtension(tagging.TaggingExtension):

    @staticmethod
    def defaultConfig():
        config = tagging.TaggingExtension.defaultConfig()
        config['automatic_keys'] = ([], "List of the hard-coded (in tagging.py) keys containing first the type of the automatic key "\
                                        "(currently only based on the folder name, or the file name), "\
                                        ", then the name to use to display each of these, and finally the allowed values. "\
                                        "If no allowed values are specified, all values found are allowed.")

        # Disable by default
        config['active'] = (False, config['active'][1])
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, SourceTaggingCommand())

    def __init__(self, *args, **kwargs):
        tagging.TaggingExtension.__init__(self, *args, **kwargs)
        self._automatic_keys = self['automatic_keys']

    @property
    def automatic_keys(self):
        return self._automatic_keys


class SourceTaggingCommand(tagging.TaggingCommand):
    COMMAND= 'syntax'
    # This is not ideal but we cannot share commands for now?
    SUBCOMMAND= None

    def createToken(self, parent, info, page, settings):
        """
        Process automated tags
        """

        # Get basic meta data for the tagging
        mpath=re.sub(r'^.*?moose/', 'moose/', page.source)
        name = mpath.split("/")[-1].replace(".md", "")

        good_keys=[]

        # Add automatic tagging with some simple data extracted from file path/name. Notably for:
        # - system or modules, through the folder name
        # - discretization or data type, if known from the name (for now)
        # Note: - we only add the key if the hard-coded search succeeds
        #       - we do not support more than one match per key
        #       - we do not override !tag output (if using same js file)
        for data_raw in self.extension.automatic_keys:

            # Parse the specifications of the automatic keys
            data = data_raw.split(' ')
            if data == []:
                break
            key = data[0]
            name_to_use = data[1]
            start_expected = 2
            if (len(data) > start_expected):
                expected_values = data[start_expected:]
            else:
                expected_values = []

            # Examine tag data in light of the hard-coded tagging keys, append if correct
            if key == "system":
                system = mpath.split("source/")[-1].split("/")[0]
                if (expected_values == []) or (system in expected_values):
                    good_keys.append([name_to_use, system])
            elif key == "module":
                module = mpath.split("/doc/")[0].split("/")[-1]
                if (expected_values == []) or (module in expected_values):
                    good_keys.append([name_to_use, module])
            elif key == "name_affix":
                for name_affix in expected_values:
                    if name_affix in name:
                        if (expected_values == []) or (name_affix in expected_values):
                            if (name_to_use not in [item[0] for item in good_keys]):
                                good_keys.append([name_to_use, name_affix])
            else:
                LOG.error("Unrecognized automatic tagging key:", key)

        # Only add to tag database if 'name' is provided
        if len(name) != 0:
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

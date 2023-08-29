# Writing an extension from scratch
import os, re
import logging
from ..tree import tokens
from ..common import __init__
from . import command

"""
This extension defines the tagger command: !tagger name path key:value.  Tagger will except a string
that represents the markdown file that is associated with an arbitrary list of key:value pairs. Arbitrary
spacing is allowed after the name/markdown name, however only one space is allowed before the name/markdown.
Ex: '!tagger name    k1:v1  ka:va thing1:thing2' is okay, but not '!tagger  name'.

Tagger checks that all linked moose pages are unique and will not allow duplicate names in the
dictionary.  Duplicate key value pairs are allowed.

Example Tagger command in *.md:
!tagger geochem keyg:valg keychem:valuechem

Example Output TagDictionary:
{"data":
[{"name": "heatconduction", "path": "moose/modules/heat_conduction/doc/content/modules/heat_conduction/index.md", "key_vals": {"keyheat": "valheat", "key": "val", "key1": "val1"}},
{"name": "index", "path": "moose/modules/doc/content/index.md", "key_vals": {"key1": "val1", "keya": "val"}},
{"name": "index2", "path": "moose/modules/doc/content/index2.md", "key_vals": {"key1": "val1", "keya": "val", "thing1": "thing2"}},
{"name": "geochem", "path": "moose/modules/geochemistry/doc/content/modules/geochemistry/index.md", "key_vals": {"keyg": "valg", "keychem": "valuechem"}},
{"name": "vortex", "path": "moose/modules/level_set/doc/content/modules/level_set/example_vortex.md", "key_vals": {"keyvor": "valvor", "key": "val", "key1": "val1"}}]
}
"""

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):
    return TaggingExtension(**kwargs)

class TaggingExtension(command.CommandExtension):

    @staticmethod
    def defaultConfig():
        config = command.CommandExtension.defaultConfig()
        config['allowed_keys'] = ([],
                                  "List of tag keys allowed in documentation pages. If empty, all " \
                                  "keys allowed.")
        return config

    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, TaggingCommand())

    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self._database={'data':[]}
        self._allowed_keys = self['allowed_keys']
        LOG.info(self._allowed_keys)

    @property
    def database(self):
        return self._database

    @property
    def allowed_keys(self):
        return self._allowed_keys

class TaggingCommand(command.CommandComponent):
    COMMAND= 'tagger'
    SUBCOMMAND= '*'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings

    def createToken(self, parent, info, page, settings):
        name=info[2]
        keylist=info[3].split()
        mpath=re.sub(r'^.*?moose/', 'moose/', page.source)
        EntryKeyValDict=[]
        for keys in keylist:
            key_vals=keys.split(':')
            EntryKeyValDict.append([key_vals[0],key_vals[1]])

        goodkeys=[]
        for pair in EntryKeyValDict:
            if pair[0] not in self.extension.allowed_keys:
                msg = "Not an Allowed Key; not adding 'key' to dictionary: "
                msg += pair[0]
                LOG.warning(msg)
            else:
                goodkeys.append([pair[0], pair[1]])

        PageData= {'name':name, "path":mpath, "key_vals":dict(goodkeys)}

        if len(self.extension.database['data'])>0 and PageData['name'] in self.extension.database['data'][0]['name']:
            msg = "Tag already exists; not adding 'name' to dictionary: "
            msg += PageData['name']
            LOG.warning(msg)
        else:
            self.extension.database['data'].append(PageData)

        tag_dict_str=str(self.extension.database)
        key_list_regex=self.extension.allowed_keys+['data','name', 'path', 'key_vals']
        for i in range(len(key_list_regex)):
            regex_replace=f"'{key_list_regex[i]}':"
            tag_dict_str=re.sub(regex_replace,key_list_regex[i]+':', tag_dict_str)
        tag_dict_str=re.sub("\s","", tag_dict_str)

        # regex to replace any old dict of this format: \{data: \[{.+}}]}
        #my path to Derek's static file: /Users/rogedd/projects/tagview/dist/assets/index-93b559a6.js
        static_path='/Users/rogedd/projects/tagview/dist/assets/index-93b559a6.js'
        with open(static_path,'r') as f:
            content=f.readlines()
        content[-1]=re.sub('\{data:\[\{.+\}\}\]\}',str(tag_dict_str), content[-1])
        with open(static_path, 'w') as f:
            f.writelines(content)
            f.close()

        return(info)

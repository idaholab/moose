# Writing an extension from scratch
import os, re
import pickle
import logging
from ..tree import tokens 
from ..common import __init__ 
from . import command 
import json
import codecs
"""     Tagger ouputs to tags.txt found at: 'moose/python/MooseDocs/extensions'

        This extension defines the tagger command: !tagger name path key:value.  Tagger will except a string that represents the markdown file that is associated with an arb. list of key:value pairs.
        Arb spacing is allowed after the name/markdown name, however only one space is allowed before the name/markdown.  Ex: !tagger name    k1:v1  ka:va thing1:thing2 is okay, but not
        !tagger  name.
        
        Tagger checks that all linked moose pages are unique and will not allow duplicate namess in the dictionary.  Duplicate key value pairs are allowed.
        
        If moose is served and *.md is editied to change an existing !tagger command, recompiling will NOT pick up the change since the moose page is already in the dictionary.  To fix this rm tags.txt found 
        in the 'moose/python/MooseDocs/extensions' and create a new one: vim tags.txt.  After compiling with the empty txt the changes will be up dated. Probably a best practice to do this before using the 
        pkl to generate the database filtering system. 
        
        Since tagger & database happen before moose is served !tagger in *.md must be saved before ./moosedocs.py build --serve for the name to appear in the database filtering system. However, saving
        *.md with new !tagger will add it to the tags.txt.   
        
        Example Tagger command in *.md:
        !tagger geochem keyg:valg keychem:valuechem

        Example Output TagDictionary in tags.txt:
        {"data": 
        [{"name": "heatconduction", "path": "moose/modules/heat_conduction/doc/content/modules/heat_conduction/index.md", "key_vals": {"keyheat": "valheat", "key": "val", "key1": "val1"}}, 
        {"name": "index", "path": "moose/modules/doc/content/index.md", "key_vals": {"key1": "val1", "keya": "val"}}, 
        {"name": "index2", "path": "moose/modules/doc/content/index2.md", "key_vals": {"key1": "val1", "keya": "val", "thing1": "thing2"}}, 
        {"name": "geochem", "path": "moose/modules/geochemistry/doc/content/modules/geochemistry/index.md", "key_vals": {"keyg": "valg", "keychem": "valuechem"}}, 
        {"name": "vortex", "path": "moose/modules/level_set/doc/content/modules/level_set/example_vortex.md", "key_vals": {"keyvor": "valvor", "key": "val", "key1": "val1"}}]
        }  """

LOG = logging.getLogger(__name__)

def make_extension(**kwargs):  #reqired extension
    return TaggingExtension(**kwargs)

TaggingToken = tokens.newToken('TaggingToken', brand='')
TaggingTitle = tokens.newToken('TaggingTitle', brand='', prefix=True, center=False, icon=True, icon_name=None)
TaggingContent = tokens.newToken('TaggingContent')

class TaggingExtension(command.CommandExtension):
    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, TaggingCommand())
    def __init__(self, *args, **kwargs):
        command.CommandExtension.__init__(self, *args, **kwargs)
        self._database={'data':[]}
    @property
    def database(self):
        return self._database
        

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
            
        PageData= {'name':name, "path":mpath, "key_vals":dict(EntryKeyValDict)}
        # UploadableEntry={'data':[]}
        # UploadableEntry['data'].append(PageData)
        # print('\n\n\n\n')
        # print('UploadableEntry')
        # print(UploadableEntry)
        # print('\n')
        
        # # Set storage location
        while "moose" not in os.listdir():
            os.chdir('../')
        os.chdir('moose/python/MooseDocs/extensions')
        
        # save the dictionary 
        first_flag=0  # if there are no elements in dict then logic is needed to include 1st entry since  TagDictionary == listed
        existing_dictionary=0
        try:
            with open('tags.txt') as f:
                TagDictionary=json.load(f)

            # with open('tags.pkl', 'rb') as f:
            #     TagDictionary= pickle.load(f)
        except:
            # if len(TagDictionary.keys())<1:
            UploadableEntry={'data':[]}
            UploadableEntry['data'].append(PageData)
            TagDictionary = UploadableEntry
            first_flag+=1
        else:
            for d in TagDictionary['data']:
                if PageData['name'] in d['name']:
                    existing_dictionary+=1
                    if existing_dictionary ==1 and first_flag==0:
                        msg = "Tag already exists; not adding to 'name' dictionary: "
                        msg += PageData['name']
                        LOG.warning(msg)
                    break



        # self.extension.database['data'].append(PageData)
        # print("DATABASE")
        # print(self.extension.database)
        # if not in set or first then save the new dict
        if existing_dictionary ==0 or first_flag==1:
            if first_flag ==0: # Dont want to append the 1st entry twice
                TagDictionary['data'].append(PageData)
            # print('\nTagDictionary :\n',TagDictionary)
            # print('new Tag for names\n')

            # with open('tags.pkl', 'wb') as f:
            #     pickle.dump(TagDictionary, f)

            with open('tags.txt', 'wb') as f:
                json.dump(TagDictionary, codecs.getwriter('utf-8')(f), ensure_ascii=False)
            
        return info

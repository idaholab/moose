# Writing an extension from scratch
import os
import pickle
from ..tree import tokens 
from ..common import __init__ 
from . import command 

"""     Tagger ouputs to tags.pkl found at: 'moose/python/MooseDocs/extensions'

        This extension defines the tagger command: !tagger moosepage  key:value.  Tagger will except a string that represents the markdown file that is associated with an arb. list of key:value pairs.
        Arb spacing is allowed after the moosepage/markdown name, however only one space is allowed before the moosepage/markdown.  Ex: !tagger moosepage     k1:v1  ka:va thing1:thing2 is okay, but not
        !tagger  moosepage.
        
        Tagger checks that all linked moose pages are unique and will not allow duplicate pages in the dictionary.  Duplicate key value pairs are allowed.
        
        If moose is served and *.md is editied to change an existing !tagger command, recompiling will NOT pick up the change since the moose page is already in the dictionary.  To fix this rm tags.pkl found 
        in the 'moose/python/MooseDocs/extensions' and create a new one: vim tags.pkl.  After compiling with the empty pkl the changes will be up dated. Probably a best practice to do this before using the 
        pkl to generate the database filtering system. 
        
        Since tagger & database happen before moose is served !tagger in *.md must be saved before ./moosedocs.py build --serve for the moosepage to appear in the database filtering system. However, saving
        *.md with new !tagger will add it to the tags.pkl.   
        
        Example Tagger command in *.md:
        !tagger moose/modules/geochemistry/doc/content/modules/geochemistry/index.md keyg:valg keychem:valuechem

        Example Output TagDictionary in tags.pkl:
        {'data': [
            {'MoosePage': 'moose/modules/doc/content/index.md', 'key_vals': {'key1': 'val1', 'keya': 'val'}}, 
            {'MoosePage': 'moose/modules/heat_conduction/doc/content/modules/heat_conduction/index.md', 'key_vals': {'keyheat': 'valheat', 'key': 'val', 'key1': 'val1'}}, 
            {'MoosePage': 'moose/modules/level_set/doc/content/modules/level_set/example_vortex.md', 'key_vals': {'keyvor': 'valvor', 'key': 'val', 'key1': 'val1'}}, 
            {'MoosePage': 'moose/modules/geochemistry/doc/content/modules/geochemistry/index.md', 'key_vals': {'keyg': 'valg', 'keychem': 'valuechem'}}, 
            {'MoosePage': 'moose/modules/doc/content/index2.md', 'key_vals': {'key1': 'val1', 'keya': 'val', 'thing1': 'thing2'}}
            ]}  """

def make_extension(**kwargs):  #reqired extension
    return TaggingExtension(**kwargs)

TaggingToken = tokens.newToken('TaggingToken', brand='')
TaggingTitle = tokens.newToken('TaggingTitle', brand='', prefix=True, center=False, icon=True, icon_name=None)
TaggingContent = tokens.newToken('TaggingContent')

class TaggingExtension(command.CommandExtension):
    def extend(self, reader, renderer):
        self.requires(command)
        self.addCommand(reader, TaggingCommand())


class TaggingCommand(command.CommandComponent):
    COMMAND= 'tagger'
    SUBCOMMAND= '*'

    @staticmethod
    def defaultSettings():
        settings = command.CommandComponent.defaultSettings()
        return settings
    
    def createToken(self, parent, info, page, settings):
        MoosePage=info[2]
        keylist=info[3].split()
        EntryKeyValDict=[]
        for keys in keylist:
            key_vals=keys.split(':')
            EntryKeyValDict.append([key_vals[0],key_vals[1]])
            
        PageData= {'MoosePage':MoosePage, "key_vals":dict(EntryKeyValDict)}
        UploadableEntry={'data':[]}
        UploadableEntry['data'].append(PageData)
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
            with open('tags.pkl', 'rb') as f:
                TagDictionary= pickle.load(f)
        except:
            # if len(TagDictionary.keys())<1:
            TagDictionary = UploadableEntry
            first_flag+=1
        else:
            for d in TagDictionary['data']:
                if PageData['MoosePage'] in d['MoosePage']:
                    existing_dictionary+=1
                    if existing_dictionary ==1 and first_flag==0:
                        print('Existing / Not Adding Tag to MoosePage Dictionary: \n', PageData['MoosePage'])
                    break

        # if not in set or first then save the new dict
        if existing_dictionary ==0 or first_flag==1:
            if first_flag ==0: # Dont want to append the 1st entry twice
                TagDictionary['data'].append(PageData)
            # print('\nTagDictionary :\n',TagDictionary)
            # print('new Tag for MoosePages\n')

            with open('tags.pkl', 'wb') as f:
                pickle.dump(TagDictionary, f)
            
        return info
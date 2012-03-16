import nsiqcppstyle_util
import os
import urllib
try:
    import hashlib
    md5_constructor = hashlib.md5
except ImportError:
    import md5
    md5_constructor = md5.new

url = "http://nsiqcppstyle.appspot.com"

def Update(currentVersion):
    import httplib
    import urllib2
    systemKey = nsiqcppstyle_util.GetSystemKey()
    # Get the latest version info
    try:
        print 'Update: checking for update'
        print url + "/update/" + systemKey
        request = urllib2.urlopen(url + "/update/" + systemKey)
        response = request.read()
    except urllib2.HTTPError, e:
        raise Exception('Unable to get latest version info - HTTPError = ' + str(e))

    except urllib2.URLError, e:
        raise Exception('Unable to get latest version info - URLError = ' + str(e))

    except httplib.HTTPException, e:
        raise Exception('Unable to get latest version info - HTTPException')

    except Exception, e:
        raise Exception('Unable to get latest version info - Exception = ' + str(e))

    updateInfo = None
    import updateagent.minjson
    try:
        updateInfo = updateagent.minjson.safeRead(response)
    except Exception, e:
        print e
        raise Exception('Unable to get latest version info. Try again later.')

    if Version(updateInfo['version']) > Version(currentVersion):
        print 'A new version is available.'

        # Loop through the new files and call the download function
        for agentFile in updateInfo['files']:

            #win32str = agentFile["name"].replace("/", "\\")
            eachFileName = agentFile["name"];
            if (eachFileName.endswith(".dll") or eachFileName.endswith(".zip") or eachFileName.endswith(".exe")) :
                continue
            filestr = os.path.join(nsiqcppstyle_util.GetRuntimePath(), agentFile["name"])

            if os.path.exists(filestr) :
                checksum = md5_constructor()
                f = file(filestr, 'rb').read()
                checksum.update(f)
                if agentFile["md5"] == checksum.hexdigest() :
                    continue

            agentFile['tempFile'] = DownloadFile(url, agentFile, systemKey)
            if agentFile['tempFile'] == None :
                print "Update Failed while downloading : " + agentFile['name']
                return
            agentFile['new'] = True
        import shutil
        runtimePath = nsiqcppstyle_util.GetRuntimePath()

        for agentFile in updateInfo['files']:
            eachFileName = agentFile["name"];
            if (eachFileName.endswith(".dll") or eachFileName.endswith(".zip") or eachFileName.endswith(".exe")) :
                continue
            if agentFile.get('new',None) != None :
                print 'Updating ' + agentFile['name']
                newModule = os.path.join(runtimePath, agentFile['name'])

                try:
                    if os.path.exists(newModule):
                        os.remove(newModule)
                        basedirname = os.path.dirname(newModule)
                    if not os.path.exists(basedirname) :
                        os.makedirs(basedirname)
                    shutil.move(agentFile['tempFile'], newModule)
                except OSError, e:
                    pass
        return True
    return False

def DownloadFile(url, agentFile, systemKey, recursed = False):
    print 'Downloading ' + agentFile['name']
    downloadedFile = urllib.urlretrieve(url + '/update/' + systemKey +"/" + agentFile['name'])

    checksum = md5_constructor()

    f = file(downloadedFile[0], 'rb')
    part = f.read()
    checksum.update(part)
    f.close()
    # Do we have a match?
    if checksum.hexdigest() == agentFile['md5']:
        return downloadedFile[0]
    else:
        # Try once more
        if recursed == False:
            DownloadFile(url, agentFile, systemKey, True)
        else:
            print agentFile['name'] + ' did not match its checksum - it is corrupted. This may be caused by network issues so please try again in a moment.'
            return None

import string, re
from types import StringType

class Version :
    component_re = re.compile(r'(\d+ | [a-z]+ | \.)', re.VERBOSE)

    def __init__ (self, vstring=None):
        if vstring:
            self.parse(vstring)


    def parse (self, vstring):
        # I've given up on thinking I can reconstruct the version string
        # from the parsed tuple -- so I just store the string here for
        # use by __str__
        self.vstring = vstring
        components = filter(lambda x: x and x != '.',
                            self.component_re.split(vstring))
        for i in range(len(components)):
            try:
                components[i] = int(components[i])
            except ValueError:
                pass

        self.version = components


    def __str__ (self):
        return self.vstring


    def __repr__ (self):
        return "LooseVersion ('%s')" % str(self)


    def __cmp__ (self, other):
        if isinstance(other, StringType):
            other = Version(other)

        return cmp(self.version, other.version)




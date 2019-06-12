import os

class FileChecker(object):
    """ Class that checks files and stores last-modified times. """

    def __init__(self):
        """ Establish new dictionaries for files and their corresponding modified times. """
        self.__original_times = dict()
        self.__new_times = dict()


    def getOriginalTimes(self):
        return self.__original_times

    def getNewTimes(self):
        return self.__new_times

    # Added by the MOOSE group
    def get_all_files(self, job, times):
        """ Method to get the names and last_modified_times of all files within current test location """
        for dirpath, dirnames, filenames in os.walk(job.getTestDir(), followlinks=True):
            for file in filenames:
                fullyQualifiedFile = os.path.join(dirpath, file)
                try:
                    lastModifiedTime = os.path.getmtime(fullyQualifiedFile)
                    times[fullyQualifiedFile] = lastModifiedTime
                except:
                    pass
        return times

    # Added by the MOOSE group
    def check_changes(self, originalTimes, newTimes):
        """ Method to compare names of times kept kept in the two dictionaries created filled by get_all_files """
        changed = []
        # Are the new mod times different?
        # Check if the key is there before accessing
        for fullyQualifiedFile in originalTimes:
            try:
                if originalTimes[fullyQualifiedFile] != newTimes[fullyQualifiedFile]:
                    changed.append(fullyQualifiedFile)
            except:
                pass

        ##Going to need to check to see if any other items were added
        for fullyQualifiedFile in newTimes:
            if not fullyQualifiedFile in originalTimes:
                changed.append(fullyQualifiedFile)
        return changed

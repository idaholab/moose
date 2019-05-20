import os

def get_all_files(self, job, times):
#    if foldname != os.path.curdir:
#        foldname = os.path.join(os.path.curdir, foldname)
#        print(type(jobs))
#        for job in jobs:
#            print(job)
#            print(type(job))
    files = os.listdir(job.getTestDir())
    for file in files:
        lastModifiedTime = os.path.getmtime(os.path.join(job.getTestDir(), file))

    for dirpath, dirnames, filenames in os.walk(job.getTestDir(), followlinks=True):
        #if os.path.isdir(os.path.join(job.getTestDir(), file)):
        for file in filenames:
            key = os.path.join(dirpath, file)
#                    times.update(key)
            times[key] = lastModifiedTime
    return times


def check_changes(self, originalTimes, newTimes):
    ### Are the new mod times different?
    changed = []
    ### Are some files being deleted after they were recorded by originalTimes?
    #keysWithModifiedTimes = [key for key in originalTimes if originalTimes[key] != newTimes[key]]
    ########## Check if the key is there before accessing
    for key in originalTimes:
        try:
            if originalTimes[key] != newTimes[key]:
                changed.append(key)
        except:
            print("File deleted?")

    # With the above try, we can add to changed directly
#        for key in keysWithModifiedTimes:
#            print(key, ':', originalTimes[key], '->', newTimes[key])
#            changed.append(key)

#        print(changed)

##Going to need to check to see if any other items were added

    for key in newTimes:
        if not key in originalTimes:
#                print(key, newTimes[key])
            changed.append(key)
#        print(changed)
    return changed

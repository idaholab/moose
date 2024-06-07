#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os


# Open every single source file and add the tag
for folder, subs, files in os.walk("../"):
    for file in files:
        file_path = os.path.join(folder, file)
        if ".md" in file_path:
            if "source" in file_path:
                file1 = open(file_path, 'a')
                file1.write("\n!tag name=" + file.split(".md")[0] + "\n")
                file1.close()

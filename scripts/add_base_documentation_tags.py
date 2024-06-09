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
            if "/source/" in file_path and file != "index.md":
                # Get the system and module from where we expect it in the path
                module = file_path.split("/doc/")[0].split("/")[-1]
                system = file_path.split("source/")[-1].split("/")[0]

                # additional checks to avoid messing up the repository
                file0 = open(file_path, 'r')
                lines = file0.readlines()
                add_tag = True
                # Check if the file has a title
                if ("#" not in lines[0]):
                    add_tag = False
                # Check if the file already has a tag
                for line in lines:
                    if "!tag" in line:
                        add_tag = False

                if not add_tag:
                    print("Skipping file", file_path)
                    continue
                file1 = open(file_path, 'a')
                file1.write("\n!tag name=" + file.split(".md")[0] + " module=" + module + " system=" + system + "\n")
                file1.close()

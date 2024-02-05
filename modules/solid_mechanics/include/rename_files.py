import os

initial_name = 'TensorMechanics'
final_name = 'SolidMechanics'

tests = False
source = True

# Rename files
for (dirpath, dirnames, filenames) in os.walk('./'):

    for file in filenames:

        if ((tests and (file[-2:] == '.i' or file[-5:] == 'tests')) or
            (source and (file[-2:] == '.C' or file[-2:] == '.h'))):

            if ('actions' in dirpath) and initial_name in file:

                # Get filename
                print(dirpath + '/' + file)

                os.system('git mv ' + dirpath + '/' + file + ' ' + dirpath + '/' + file.replace(initial_name, final_name))

#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from __future__ import print_function
import os

class ImageDiffer(object):
    """
    A class for comparing images using the structural similarity index (SSIM).

    https://en.wikipedia.org/wiki/Structural_similarity

    Args:
      file1[str]: The base file to compare against (gold).
      file2[str]: The file to be compared.

    Kwargs:
      allowed[float]: (Default: 0.95) The allowed lower limit of the SSIM (1 is identical images).
    """

    def __init__(self, file1, file2, **kwargs):

        # Store the file names
        self.__files = [file1, file2]

        # Extract the optional arguments
        self.__allowed = float(kwargs.pop('allowed', 0.95))

        # Storage for error messages, each stored as a tuple: (error, message)
        self.__error = 0 # The computed error
        self.__errors = []

        # Read the image files
        self.__data = []
        self.__data.append(self.__readImage(self.__files[0]))
        self.__data.append(self.__readImage(self.__files[1]))

        # Perform comparison
        self.__compare()

    def fail(self):
        """
        Check the comparison status. (public)

        Returns:
           bool: True when the test failed.
        """
        return len(self.__errors) > 0

    def message(self, **kwargs):
        """
        Print the error message(s). (public)

        Returns:
          str: The output message as a single string.
        """

        # Header
        output = []
        output.append('Running ImageDiffer.py')
        output.append('            File 1: ' + self.__files[0])
        output.append('            File 2: ' + self.__files[1])
        output.append('    Allowed (SSIM): ' + str(self.__allowed))
        output.append('   Computed (SSIM): ' + str(self.__error))
        output.append('     No. of errors: ' + str(len(self.__errors)))

        # Errors
        cnt = 0
        for e in self.__errors:
            cnt += 1
            output.append('')
            output.append('ERROR ' + str(cnt) + ':')
            output.append('  ' + e[0])
            if e[1]:
                output.append('    ' + e[1])

        # Print the output
        if kwargs.pop('output', False):
            print('\n'.join(output))

        # Return the text, as a single string
        return '\n'.join(output)
        # Errors
        cnt = 0
        for e in self.__errors:
            cnt += 1
            output.append('')
            output.append('ERROR ' + str(cnt) + ':')
            output.append('  ' + e[0])
            if e[1]:
                output.append('    ' + e[1])

        # Print the output
        if kwargs.pop('output', False):
            print('\n'.join(output))

        # Return the text, as a single string
        return '\n'.join(output)

    def __compare(self):
        """
        Perform image comparison. (private)
        """

        # Do nothing if something failed to open
        if len(self.__errors) > 0:
            return

        # Check sizes
        if (self.__data[0].size != self.__data[1].size):
            err = 'The two images are different sizes'
            msg  =  ['  File 1: ' + self.__files[0]]
            msg +=  ['    size: ' + str(self.__data[0].size)]
            msg +=  ['  File 2: ' + self.__files[1]]
            msg +=  ['    size: ' + str(self.__data[1].size)]
            self.__addError(err, msg)
            return

        # Compute the error
        import skimage.measure
        self.__error = skimage.measure.compare_ssim(self.__data[0], self.__data[1], multichannel=True)

        # Report the error
        if self.__error < self.__allowed:
            err = 'The files are different.'
            msg = ['The difference of the images exceeds the "allowed" SSIM.']
            msg += ['                 Allowed (SSIM): ' + str(self.__allowed)]
            msg += ['                Computed (SSIM): ' + str(self.__error)]
            msg += ['                Rel. difference: ' + str( abs(self.__allowed - self.__error) / self.__error)]

            self.__addError(err, msg)
            return

    def __readImage(self, filename):
        """
        A read function that appends an error message if the read fails. (private)
        """

        if not os.path.exists(filename):
            self.__addError('Failed to open ' + filename + ', the file does not exist.')
            return None

        import matplotlib.pyplot as plt
        return plt.imread(filename)

    def __addError(self, err, msg=[]):
        """
        Add an ImageError object to the storage vector (private)

        Args:
          err[str]: A string containing the error message.
          msg[list]: A detailed message for the error.
        """
        self.__errors.append((err, '\n'.join(msg)))


# This file is executable and allows for running the ImageDiffer via the command line
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(description='Perform comparison of images.')
    parser.add_argument('files', type=str, nargs='+', help="The image(s) to compare. If a single image is provided the 'gold' version is used.")
    args = parser.parse_args()

    # Test files
    n = len(args.files)
    if n == 1:
        file1 = args.files[0]
        file0 = os.path.join(os.path.dirname(file1), 'gold', os.path.basename(file1))
    elif n == 2:
        file0 = args.files[0]
        file1 = args.files[1]
    else:
        print("You must specify one or two files for comparison, see -h")

    d = ImageDiffer(file0, file1)
    print('\n\n' + d.message())

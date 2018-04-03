#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from mooseutils import message
from PyQt5 import QtCore

def setAppInformation(app_name="peacock", force=False):
    """
    Set the application depending on whether we are testing
    """
    QtCore.QCoreApplication.setOrganizationName("IdahoLab")
    QtCore.QCoreApplication.setOrganizationDomain("inl.gov")
    if message.MOOSE_TESTING_MODE:
        if force or not QtCore.QCoreApplication.applicationName():
            # We don't want to override the application name if it is already
            # set since PeacockApp will set it to "peacock_peacockapp" and
            # we want the names to be more unique.
            QtCore.QCoreApplication.setApplicationName("test_%s" % app_name)
    else:
        QtCore.QCoreApplication.setApplicationName(app_name)

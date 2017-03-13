from mooseutils import message
from PyQt5 import QtCore

def setAppInformation(app_name="peacock"):
    """
    Set the application depending on whether we are testing
    """
    QtCore.QCoreApplication.setOrganizationName("IdahoLab")
    QtCore.QCoreApplication.setOrganizationDomain("inl.gov")
    if message.MOOSE_TESTING_MODE:
        QtCore.QCoreApplication.setApplicationName("test_%s" % app_name)
    else:
        QtCore.QCoreApplication.setApplicationName(app_name)

#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtGui import QIcon
from PyQt5.QtWidgets import QApplication
from . PeacockMainWindow import PeacockMainWindow
import argparse, os, sys
from .utils import qtutils

class PeacockApp(object):
    """
    Main Peacock application.
    Builds the Qt main window and processes any command line arguments.
    """
    def __init__(self, args, qapp=None, **kwds):
        """
        Constructor.
        Takes a QApplication in the constructor to allow for easier testing with unittest.

        Input args:
            args: Command line arguments
            qapp: QApplication
        """
        super(PeacockApp, self).__init__(**kwds)
        # don't include args[0] since that is the executable name
        parser = argparse.ArgumentParser(description='MOOSE GUI Application')
        PeacockMainWindow.commandLineArgs(parser)
        parsed_args = parser.parse_args(args)

        peacock_dir = os.path.dirname(os.path.realpath(__file__))
        icon_path = os.path.join(peacock_dir, "icons", "peacock_logo.ico")
        if qapp is None:
            qapp = QApplication.instance()
        qapp.setWindowIcon(QIcon(icon_path))

        qtutils.setAppInformation("peacock_peacockapp")

        if parsed_args.exodus or parsed_args.postprocessors or parsed_args.vectorpostprocessors:
            # If the user wants to view files then don't try to automatically find an executable.
            # This should help speed up startup times.
            parsed_args.exe_search = False

        self.main_widget = PeacockMainWindow()
        self.main_widget.initialize(parsed_args)
        self.main_widget.show()
        self.main_widget.raise_()

        input_plugin = self.main_widget.tab_plugin.InputFileEditorWithMesh
        tree = input_plugin.InputFileEditorPlugin.tree
        exe_plugin = self.main_widget.tab_plugin.ExecuteTabPlugin
        exodus_plugin = self.main_widget.tab_plugin.ExodusViewer
        pp_plugin = self.main_widget.tab_plugin.PostprocessorViewer
        vpp_plugin = self.main_widget.tab_plugin.VectorPostprocessorViewer

        # issue #9255
        # For some unknown reason, the Execute tab doesn't work
        # properly on Mac low resolution displays (and some widgets
        # on the input tab ).
        # If you switch to the ExodusViewer tab then back again, it works.
        # If the Execute tab is created after the ExodusViewer
        # tab, it works. If the VTKWindowPlugin of the ExodusViewer
        # tab is removed, it works. So there is some resizing issue
        # or something.
        # This ugly hack seems to fix the immediate problem.
        if sys.platform == 'darwin':
            for idx in range(self.main_widget.tab_plugin.count()):
                self.main_widget.tab_plugin.setCurrentIndex(idx)

        if parsed_args.vectorpostprocessors:
            self.main_widget.setTab(vpp_plugin.tabName())
        elif parsed_args.postprocessors:
            self.main_widget.setTab(pp_plugin.tabName())
        elif parsed_args.exodus:
            self.main_widget.setTab(exodus_plugin.tabName())
        elif tree.app_info.valid():
            if tree.input_filename and parsed_args.auto_run:
                self.main_widget.setTab(exe_plugin.tabName())
                # These processEvents() seem to be needed on linux so
                # that the ExodusViewer gets updated properly
                qapp.processEvents()
                exe_plugin.ExecuteRunnerPlugin.runClicked()
                qapp.processEvents()
                self.main_widget.setTab(exodus_plugin.tabName())
                qapp.processEvents()
            else:
                self.main_widget.setTab(input_plugin.tabName())
        else:
            self.main_widget.setTab(exe_plugin.tabName())
        self.main_widget.setPythonVariable("PeacockApp", self)

    def run(self, qapp):
        """
        Just calls the QApplication.exec().
        Broken out to a separate function to allow for easier testing.
        """
        ret = qapp.exec_()
        qapp.quit()
        return ret

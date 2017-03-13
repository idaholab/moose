import os, contextlib
from mooseutils.ImageDiffer import ImageDiffer
from mooseutils import message
import qtutils
import time
import unittest
import glob
from PyQt5 import QtCore, QtWidgets
from peacock import PeacockApp

def find_moose_test_exe(dirname="test", exe_base="moose_test"):
    moose_dir = os.environ.get("MOOSE_DIR", "")
    method = os.environ.get("METHOD", "opt")
    if not moose_dir:
        raise Exception("MOOSE_DIR needs to be set")
    path = os.path.join(moose_dir, dirname, "%s-%s" % (exe_base, method))
    if not os.path.exists(path):
        raise Exception("%s does not exist" % path)
    return os.path.abspath(path)

def get_chigger_input(filename):
    """
    Helper for building path to a ExodusII file for testing Peacock ExodusViewer.
    """
    return os.path.join(os.environ['MOOSE_DIR'], 'python', 'chigger', 'tests', 'input', filename)

def get_chigger_input_list(*filenames):
    """
    Helper for building a list of paths to ExodusII files for testing Peacock ExodusViewer.
    """
    return [get_chigger_input(x) for x in filenames]

@contextlib.contextmanager
def remember_cwd():
    curdir= os.getcwd()
    try:
        yield
    finally:
        os.chdir(curdir)

def gold_diff(filename, allowed=0.95):
    path = os.path.abspath(filename)
    filename_only = os.path.basename(path)
    base_dir = os.path.dirname(path)
    goldname = os.path.join(base_dir, "gold", filename_only)
    differ = ImageDiffer(goldname, path, allowed=allowed)
    print(differ.message())
    return differ.fail()


def checkFile(output, gold_file, write_output=False):
    if write_output:
        with open("tmp_out.txt", "w") as f:
            f.write(output)

    with open(gold_file, "r") as f:
        gold_output = f.read()
        return gold_output == output

def compareFiles(test_file, gold_file):
    with open(test_file, "r") as f:
        test_output = f.read()

    with open(gold_file, "r") as f:
        gold_output = f.read()
    return gold_output == test_output

def set_window_size(vtk_window, size=[640,640]):
    vtk_window.setFixedSize(QtCore.QSize(*size))

def process_events(qapp, t=1):
    start = time.time()
    end = start + t
    while(time.time() < end):
        qapp.processEvents()

def remove_file(filename):
    try:
        path = os.path.abspath(filename)
        os.remove(path)
    except:
        pass

def clean_files():
    remove_file("out_transient.csv")
    remove_file("oversample_oversample2.e")
    remove_file("out_transient.e")
    remove_file("peacock_run_exe_tmp.i")
    remove_file("peacock_run_tmp.i")
    remove_file("peacock_run_tmp_mesh.e")
    remove_file("peacock_run_tmp_mesh.i")
    for i in range(21):
        remove_file("peacock_run_tmp_out_line_sample_{0:04d}.csv".format(i))
        remove_file("time_data_line_sample_{0:04d}.csv".format(i))
    remove_file("time_data_line_sample_time.csv")

def run_tests():
    unittest.main(verbosity=2)

def addQObjectByName(qobject, name, matched):
    if qobject.objectName() == name:
        matched.append(qobject)

    for child in qobject.children():
        addQObjectByName(child, name, matched)

def findQObjectsByName(top_qobject, name):
    matched = []
    addQObjectByName(top_qobject, name, matched)
    return matched

def addQObjectByType(qobject, name, matched):
    full_name = "%s.%s" % (qobject.__class__.__module__, qobject.__class__.__name__)
    if full_name == name:
        matched.append(qobject)

    for child in qobject.children():
        addQObjectByType(child, name, matched)

def findQObjectsByType(top_qobject, type_name):
    matched = []
    addQObjectByType(top_qobject, type_name, matched)
    return matched

class PeacockTester(unittest.TestCase):
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        self.finished = False
        self.app = None
        message.MOOSE_TESTING_MODE = True
        qtutils.setAppInformation()
        self.starting_directory = os.getcwd()

    def tearDown(self):
        if self.app:
            self.app.main_widget.close()
            del self.app
        os.chdir(self.starting_directory)

    def run_finished(self, current, total):
        if current == total:
            self.finished = True

    def createPeacockApp(self, args):
        from peacock import PeacockApp
        self.app = PeacockApp.PeacockApp(["-size", "1024", "1024", "-w", os.getcwd()] + args, self.qapp)
        process_events(self.qapp, 4)
        self.app.main_widget.tab_plugin.ExecuteTabPlugin.ExecuteRunnerPlugin.runProgress.connect(self.run_finished)
        return self.app

    def checkImage(self, filename, allowed=0.95):
        self.assertTrue(gold_diff(filename, allowed), "Image %s does not match the gold file" % filename)

    def checkFile(self, output, gold_file, write_output=False):
        self.assertTrue(checkFile(output, gold_file, write_output), "Output does not match the gold file %s" % gold_file)

    def compareFiles(self, test_file, gold_file):
        self.assertTrue(compareFiles(test_file, gold_file), "%s does not match the gold file %s" % (test_file, gold_file))

    def fullPath(self, py_file, basename):
        dirname = os.path.dirname(py_file)
        return os.path.join(dirname, basename)

class PeacockImageTestCase(unittest.TestCase):
    """
    Base test object for unittests that require image comparisons.
    """

    @classmethod
    def setUpClass(cls):
        """
        Clean up image files.
        """
        super(PeacockImageTestCase, cls).setUpClass()

        filenames = glob.glob(cls.filename('_*.png'))
        for fname in filenames:
            os.remove(fname)

    @classmethod
    def filename(cls, basename):
        """
        Return the base name with the class name prefix.
        """
        return '{}_{}'.format(cls.__name__, basename)

    def write(self, filename, **kwargs):
        """
        Write the image file to be compared.

        NOTE: Requires self._window to be a VTKWindowWidget or FigureWidget object.
        """
        self._window.onWrite(filename, **kwargs)

    def assertImage(self, basename, allowed=0.99, **kwargs):
        """
        Create the supplied image and assert that it is same to the gold standard.
        """
        filename = type(self).filename(basename)
        goldname = os.path.join('gold', filename)

        self.write(filename, **kwargs)
        differ = ImageDiffer(goldname, filename, allowed=allowed)
        print differ.message()
        self.assertFalse(differ.fail(), "{} does not match the gold file.".format(filename))

    def sleepIfSlow(self):
        """
        Ugly hack to get testing to work on the slow linux machine.
        Some tests seem to require a sleep in between individual tests
        in a unittest class.
        """
        t = os.environ.get("PEACOCK_SLEEP_ON_SLOW_MACHINE", 0)
        time.sleep(float(t))

class PeacockAppImageTestCase(PeacockImageTestCase):
    """
    Base class for testing complete Peacock gui.
    """
    qapp = QtWidgets.QApplication([])

    def setUp(self):
        """
        Creates the peacock application.cd
        """
        args = ["-size", "1024", "768", "-w", os.getcwd(), "-i", "../../common/simple_diffusion.i", "-e", find_moose_test_exe()]
        self._app = PeacockApp.PeacockApp(args, self.qapp)
        self._window = self._app.main_widget.tab_plugin.ExodusViewer.currentWidget().VTKWindowPlugin
        set_window_size(self._window)
        remove_file('peacock_run_exe_tmp_out.e')

    def selectTab(self, tab):
        """
        Helper function for toggling tabs.
        """
        self._app.main_widget.tab_plugin.setCurrentWidget(tab)
        self._app.main_widget.tab_plugin.currentChanged.emit(self._app.main_widget.tab_plugin.currentIndex())
        process_events(self.qapp, t=1)

    def execute(self):
        """
        Helper for running executable.
        """
        execute = self._app.main_widget.tab_plugin.ExecuteTabPlugin
        execute.ExecuteRunnerPlugin.runClicked()
        execute.ExecuteRunnerPlugin.runner.process.waitForFinished()
        process_events(self.qapp, t=1)

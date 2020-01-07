#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from code import InteractiveConsole
from PyQt5.QtCore import QObject, pyqtSignal, pyqtSlot, QEvent, Qt, QSettings
from PyQt5.QtWidgets import QWidget, QPlainTextEdit, QSizePolicy
import sys
try:
    from StringIO import StringIO
except ImportError:
    from io import StringIO
from peacock.utils import WidgetUtils
from peacock.base import MooseWidget

class QPythonConsole(QObject, MooseWidget):
    """
    A python interactive interpreter that emits signals for output
    and has a slot for input, allowing to be hooked up to Qt widgets.
    Signals:
        write_output: Some output was written to the console. Argument is the output.
        prompt_changed: The prompt needs to be changed. This is for line continuation.
    """
    write_output = pyqtSignal(str)
    prompt_changed = pyqtSignal(str)

    def __init__(self, locals=None, filename="Python Console", **kwds):
        """
        Constructor.
        Input:
            locals: Local variables to start the console with. These will be accessible within the console.
            filename: Used in the python InteractiveConsole
        """
        super(QPythonConsole, self).__init__(**kwds)
        self.console = InteractiveConsole(locals, filename)

        self.current_prompt = ""
        self.more = False
        self.prompt = ""
        self._setPrompt()
        # on ubuntu this might be set to apport_excepthook which screws
        # up InteractiveConsole
        sys.excepthook = sys.__excepthook__
        # readline support doesn't work
        # try:
        #     import readline
        # except ImportError:
        #     pass
        # else:
        #     import rlcompleter
        #     readline.set_completer(rlcompleter.Completer(self.locals).complete)
        #     readline.parse_and_bind("tab:complete")
        self.setup()

    def _setPrompt(self):
        """
        See if we are continuing a line.
        """
        if self.more:
            self.prompt = "... "
        else:
            self.prompt = ">>> "
        self.prompt_changed.emit(self.prompt)

    def write(self, data):
        """
        Emit signal of output
        """
        self.write_output.emit(data.rstrip())

    @pyqtSlot(str)
    def _newLine(self, line):
        """
        Replace stdout and stderr with StringIO to
        save output of the command.
        Input:
            line: str: line to be interpreted
        """
        old_stdout = sys.stdout
        old_stderr = sys.stderr
        output = StringIO()
        sys.stdout = output
        sys.stderr = output
        try:
            # push the line to the interpreter
            self.more = self.console.push(str(line))
        finally:
            sys.stdout = old_stdout
            sys.stderr = old_stderr
            self.write_output.emit(str(output.getvalue().rstrip()))
        self._setPrompt()

class PythonConsoleWidget(QWidget, MooseWidget):
    """
    Widget holding the input/output of a python console.

    Saves/restores history to preferences. This allows previous commands
    to be saved/restored.
    There is a global dict called "peacock" that allows saving arbitrary
    variables in.
    For example, peacock["your_variable"] could be any python variable.

    Signals:
        new_line: A new line of input. Argument is the input.
    """
    new_line = pyqtSignal(str)

    def __init__(self, **kwds):
        super(PythonConsoleWidget, self).__init__(**kwds)

        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.setLayout(self.top_layout)
        self.output = QPlainTextEdit(parent=self)
        self.output.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.output.setReadOnly(False)
        self.output.setFocusPolicy(Qt.ClickFocus)
        self.output.setStyleSheet("QPlainTextEdit { background: black; color: white;}")
        self.output.setTextInteractionFlags(Qt.TextSelectableByMouse|Qt.TextSelectableByKeyboard|Qt.LinksAccessibleByMouse|Qt.LinksAccessibleByKeyboard)
        self.top_layout.addWidget(self.output)

        self.input_layout = WidgetUtils.addLayout()
        self.top_layout.addLayout(self.input_layout)
        self.prompt = WidgetUtils.addLabel(self.input_layout, self, ">>>")

        self.input_line = WidgetUtils.addLineEdit(self.input_layout, self, self._returnPressed)
        self.input_line.setFocusPolicy(Qt.StrongFocus)
        self.input_line.installEventFilter(self)

        self.user_inputs = []
        self.current_index = -1
        # get a list of globals and locals from the callstack
        self._global_data = {}
        self._global_data['global_vars'] = globals()
        self._global_data['peacock'] = {}
        self.console = QPythonConsole(self._global_data, parent=self)
        self.console.write_output.connect(self.output.appendPlainText)
        self.output.appendPlainText("Peaock variables are in the dict 'peacock'")
        self.output.appendPlainText("Global variables are in the dict 'global_vars'")
        self.console.prompt_changed.connect(self.prompt.setText)
        self.new_line.connect(self.console._newLine)
        self.console._setPrompt()
        self._loadHistory()
        self.resize(600, 400)
        self.setup()

    def _loadHistory(self):
        """
        Loads previous commands from settings.
        """
        settings = QSettings()
        history = settings.value("python/history", type=str)
        if history != None:
            for v in history:
                self.user_inputs.append(str(v))
            self.current_index = len(self.user_inputs)

    def eventFilter(self, obj, event):
        """
        Process QEvent
        Input:
            obj: The object that the event happened on
            event: QEvent() object
        Return:
            True if we processed this event. False otherwise.
        """
        if obj == self.input_line:
            if event.type() == QEvent.KeyPress:
                if event.key() == Qt.Key_Up:
                    self._changeInput(-1)
                    return True
                elif event.key() == Qt.Key_Down:
                    self._changeInput(1)
                    return True
                elif event.key() == Qt.Key_Tab:
                    # don't allow to tab out of the command line
                    return True
        return False

    def saveHistory(self):
        """
        Save history into settings.
        """
        settings = QSettings()
        num_to_save = settings.value("python/history_size", type=int)
        if num_to_save == None:
            num_to_save = 50
            settings.setValue("python/history_size", num_to_save)

        settings.setValue("python/history", self.user_inputs[-num_to_save:])

    def _changeInput(self, change):
        """
        Looks through the history and changes the input line.
        Input:
            change: int that specifies where in the list to change to
        """
        self.current_index += change
        if self.current_index < 0:
            self.current_index = 0
        elif self.current_index >= len(self.user_inputs):
            self.current_index = len(self.user_inputs)
        if self.current_index >= 0 and self.current_index < len(self.user_inputs):
            self.input_line.setText(self.user_inputs[self.current_index ])

    def setVariable(self, name, value):
        """
        Put a variable into the global dict "peacock".
        Input:
            name: key value of the "peacock" dict
            value: The value
        """
        self._global_data["peacock"][name] = value

    @pyqtSlot()
    def _returnPressed(self):
        """
        The user pressed return so process the input line
        """
        text = str(self.input_line.text())
        self.input_line.setText("")
        self.new_line.emit(text)
        if text:
            self.user_inputs.append(text)
        self.current_index = len(self.user_inputs)

#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtWidgets, QtCore, QtGui
from peacock.utils import WidgetUtils

class MediaControlWidgetBase(object):
    """
    Base for media controls.
    """

    _icon_size = QtCore.QSize(32, 32)

    #: Emitted when the play timer is started/stopped
    playStart = QtCore.pyqtSignal()
    playStop = QtCore.pyqtSignal()

    def __init__(self):
        super(MediaControlWidgetBase, self).__init__()

        # Initiate time member variables
        self._times = None #function
        self._current_step = -1
        self._num_steps = None
        self._playing = False

        # Widget settings
        self.setEnabled(False)
        self.setStyleSheet("QGroupBox {border:0}")

        # Add the main layout for this widget
        self.MainLayout = QtWidgets.QVBoxLayout()
        self.setLayout(self.MainLayout)

        # Define a widget to contain the button objects
        self.ButtonLayout = QtWidgets.QHBoxLayout()
        self.MainLayout.addLayout(self.ButtonLayout)

        # Media control buttons
        self.__addButton('BeginButton', "Set the simulation to the beginning.", 'begin.ico')
        self.__addButton('BackwardButton', "Move simulation back one timestep.", 'backward.ico')
        self.__addButton('PlayButton', "Play through the simulation with time.", 'play.ico')
        self.__addButton('PauseButton', "Stop playing through the simulation.", 'pause.ico')
        self.__addButton('ForwardButton', "Move simulation forward one timestep.", 'forward.ico')
        self.__addButton('EndButton', "Set the simulation to the end.", 'end.ico')

        # Move the timestep/time edit boxes to the right side
        self.ButtonLayout.addStretch(1)

        # TimeStep display/edit
        self.__addEditBox('TimeStepDisplay', 'Timestep:', "Set the simulation timestep.", True)
        self.__addEditBox('TimeDisplay', 'Time:', "Set the simulation time.")
        self.__addEditBox('FrameDelayDisplay', 'Frame delay:', "Set the delay of playback, in milliseconds.", True, "100")

        # Slider
        self.TimeSlider = QtWidgets.QSlider()
        self.MainLayout.addWidget(self.TimeSlider)

        self.Timer = QtCore.QTimer()
        self.Timer.timeout.connect(self.timerUpdate)
        self.Timer.setInterval(100)

        # Call MooseWidget::setup()
        self.setup()

    def updateControls(self, **kwargs):
        """
        General callback used by all of the widgets contained within this widget.
        """
        self.setEnabled(True)

    def updateTimeDisplay(self):
        """
        Update the time display widgets.
        """

        if len(self._times) == 0:
            self.setEnabled(False)
            return
        else:
            self.setEnabled(True)

        step = self._current_step
        if step == -1:
            step = self._num_steps - 1

        if not self._playing:
            if step == 0:
                self.BackwardButton.setEnabled(False)
                self.BeginButton.setEnabled(False)
                self.ForwardButton.setEnabled(True)
                self.EndButton.setEnabled(True)

            elif step == self._num_steps - 1:
                self.BackwardButton.setEnabled(True)
                self.BeginButton.setEnabled(True)
                self.ForwardButton.setEnabled(False)
                self.EndButton.setEnabled(False)

            else:
                self.BackwardButton.setEnabled(True)
                self.BeginButton.setEnabled(True)
                self.ForwardButton.setEnabled(True)
                self.EndButton.setEnabled(True)

        self.TimeSlider.setRange(0, self._num_steps - 1)
        self.TimeSlider.setValue(step)

        if not self.TimeStepDisplay.hasFocus():
            self.TimeStepDisplay.blockSignals(True)
            self.TimeStepDisplay.setText(str(step))
            self.TimeStepDisplay.blockSignals(False)
        if not self.TimeDisplay.hasFocus():
            self.TimeDisplay.blockSignals(True)
            self.TimeDisplay.setText(str(self._times[step]))
            self.TimeDisplay.blockSignals(False)

    def start(self):
        """
        Start the play timer.
        """
        self.playStart.emit()
        self.Timer.start()

    def stop(self):
        """
        Stop the play timer.
        """
        self.playStop.emit()
        self.Timer.stop()

    def _setupPauseButton(self, qObject):
        qObject.setEnabled(False)
        qObject.setVisible(False)

    def _callbackBeginButton(self):
        self._callbackPauseButton()
        self.updateControls(timestep=0, time=None)

    def _callbackBackwardButton(self):
        self._callbackPauseButton()
        if self._current_step == -1:
            self._current_step = self._num_steps - 1
        self.updateControls(timestep=self._current_step - 1, time=None)

    def timerUpdate(self):
        timestep = self._current_step + 1
        if timestep > len(self._times) - 1:
            self._callbackPauseButton()
            return
        self.updateControls(timestep=timestep, time=None)

    def _callbackPlayButton(self):

        if self._current_step == len(self._times) - 1:
            self.BeginButton.clicked.emit(True)

        self.PauseButton.setEnabled(True)
        self.PauseButton.setVisible(True)

        self.PlayButton.setEnabled(False)
        self.PlayButton.setVisible(False)

        self.BeginButton.setEnabled(False)
        self.BackwardButton.setEnabled(False)
        self.ForwardButton.setEnabled(False)
        self.EndButton.setEnabled(False)

        self.TimeDisplay.setEnabled(False)
        self.TimeStepDisplay.setEnabled(False)
        self.TimeSlider.setEnabled(False)

        self._playing = True
        self._callbackFrameDelayDisplay()
        self.start()

    def _callbackPauseButton(self):
        self._playing = False
        self.stop()
        self.PauseButton.setEnabled(False)
        self.PauseButton.setVisible(False)

        self.PlayButton.setEnabled(True)
        self.PlayButton.setVisible(True)

        status = self._current_step > 0
        self.BeginButton.setEnabled(status)
        self.BackwardButton.setEnabled(status)

        status = self._current_step != len(self._times) - 1
        self.ForwardButton.setEnabled(status)
        self.EndButton.setEnabled(status)

        self.TimeDisplay.setEnabled(True)
        self.TimeStepDisplay.setEnabled(True)
        self.TimeSlider.setEnabled(True)

    def _callbackForwardButton(self):
        self._callbackPauseButton()
        self.updateControls(timestep=self._current_step + 1, time=None)

    def _callbackEndButton(self):
        self._callbackPauseButton()
        self.updateControls(timestep=-1, time=None)

    def _callbackTimeStepDisplay(self, text):
        self._callbackPauseButton()
        if text:
            self.updateControls(timestep=int(float(text)), time=None)

    def _callbackTimeDisplay(self, text):
        self._callbackPauseButton()
        if text:
            self.updateControls(time=float(text), timestep=None)

    def _callbackFrameDelayDisplay(self, text=""):
        text = self.FrameDelayDisplay.text()
        if text:
            self.Timer.setInterval(int(text))

    def _setupTimeSlider(self, qobject):
        qobject.setOrientation(QtCore.Qt.Horizontal)
        qobject.sliderReleased.connect(self._callbackTimeSlider)

    def _callbackTimeSlider(self):
        self.updateControls(timestep=self.TimeSlider.value(), time=None)

    def __addButton(self, name, tooltip, icon):
        qobject = QtWidgets.QPushButton(self)
        qobject.setToolTip(tooltip)
        qobject.clicked.connect(getattr(self, '_callback' + name))
        qobject.setIcon(WidgetUtils.createIcon(icon))
        qobject.setIconSize(self._icon_size)
        qobject.setFixedSize(qobject.iconSize())
        qobject.setStyleSheet("QPushButton {border:none}")
        self.ButtonLayout.addWidget(qobject)
        setattr(self, name, qobject)

    def __addEditBox(self, name, label, tooltip, int_validate=False, default=""):
        edit = QtWidgets.QLineEdit()

        if int_validate:
            validate = QtGui.QIntValidator()
        else:
            validate = QtGui.QDoubleValidator()
        validate.setBottom(0)
        edit.setValidator(validate)
        edit.setToolTip(tooltip)
        edit.setSizePolicy(QtWidgets.QSizePolicy.Expanding, QtWidgets.QSizePolicy.Maximum)
        edit.setText(default)
        edit.textChanged.connect(getattr(self, '_callback' + name))

        label = QtWidgets.QLabel(label)
        label.setAlignment(QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        label.setBuddy(edit)
        self.ButtonLayout.addWidget(label)
        self.ButtonLayout.addWidget(edit)
        setattr(self, name, edit)
        setattr(self, name + 'Label', label)

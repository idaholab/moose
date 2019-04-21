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
from mooseutils import message
from PyQt5 import QtWidgets, QtGui
from PyQt5.QtCore import Qt

def addLineEdit(layout, parent, callback, readonly=False):
    line = QtWidgets.QLineEdit(parent)
    if callback:
        line.editingFinished.connect(callback)
    if layout is not None:
        layout.addWidget(line)
    line.setReadOnly(readonly)
    return line

def addProgressBar(layout, parent, min_val=0, max_val=100, callback=None):
    bar = QtWidgets.QProgressBar(parent)
    if callback:
        bar.valueChanged.connect(callback)
    bar.setRange(min_val, max_val)
    layout.addWidget(bar)
    return bar

def addButton(layout, parent, name, callback, enabled=True):
    button = QtWidgets.QPushButton(name, parent)
    button.clicked.connect(callback)
    if layout is not None:
        layout.addWidget(button)
    button.setEnabled(enabled)
    return button

def addLabel(layout, parent, name):
    label = QtWidgets.QLabel(name, parent)
    if layout is not None:
        layout.addWidget(label)
    label.setSizePolicy(QtWidgets.QSizePolicy.Fixed, QtWidgets.QSizePolicy.Fixed)
    return label

def addLayout(vertical=False, grid=False):
    if vertical:
        layout = QtWidgets.QVBoxLayout()
        layout.setSpacing(1)
    elif grid:
        layout = QtWidgets.QGridLayout()
        layout.setSpacing(1)
    else:
        layout = QtWidgets.QHBoxLayout()
        layout.setSpacing(1)
    layout.setContentsMargins(0, 0, 0, 0)
    return layout

def addCheckbox(layout, parent, name, callback, reverse=False):
    checkbox = QtWidgets.QCheckBox(name, parent)
    if callback:
        checkbox.clicked.connect(callback)
    if reverse:
        checkbox.setLayoutDirection(Qt.RightToLeft)
    if layout is not None:
        layout.addWidget(checkbox)
    return checkbox

def addAction(menu, name, callback, shortcut=None, shortcut_app_level=False):
    action = QtWidgets.QAction(name, menu)
    action.triggered.connect(callback)
    menu.addAction(action)
    if shortcut:
        action.setShortcut(shortcut)
        if shortcut_app_level:
            action.setShortcutContext(Qt.ApplicationShortcut)
    return action

def addShortcut(parent, keys, callback, shortcut_app_level=False, shortcut_with_children=False):
    shortcut = QtWidgets.QShortcut(parent)
    shortcut.setKey(keys)
    if shortcut_app_level:
        shortcut.setContext(Qt.ApplicationShortcut)
    if shortcut_with_children:
        shortcut.setContext(Qt.WidgetWithChildrenShortcut)
    if callback:
        shortcut.activated.connect(callback)
    return shortcut


"""
Global settings grouping for storing/loading GUI state.
"""
WIDGET_SETTINGS_CACHE = dict()
WIDGET_SETTINGS_CACHE[QtWidgets.QLineEdit] = [(QtWidgets.QLineEdit.text, QtWidgets.QLineEdit.setText),
                                              (QtWidgets.QLineEdit.styleSheet, QtWidgets.QLineEdit.setStyleSheet)]
WIDGET_SETTINGS_CACHE[QtWidgets.QCheckBox] = [(QtWidgets.QCheckBox.isChecked, QtWidgets.QCheckBox.setChecked)]
WIDGET_SETTINGS_CACHE[QtWidgets.QComboBox] = [(QtWidgets.QComboBox.currentText,
                                               lambda s, t: QtWidgets.QComboBox.setCurrentIndex(s, QtWidgets.QComboBox.findText(s, t)))]
WIDGET_SETTINGS_CACHE[QtWidgets.QSlider] = [(QtWidgets.QSlider.sliderPosition, QtWidgets.QSlider.setSliderPosition)]
WIDGET_SETTINGS_CACHE[QtWidgets.QGroupBox] = [(QtWidgets.QGroupBox.isChecked, QtWidgets.QGroupBox.setChecked)]
WIDGET_SETTINGS_CACHE[QtWidgets.QSpinBox] = [(QtWidgets.QSpinBox.value, QtWidgets.QSpinBox.setValue)]
WIDGET_SETTINGS_CACHE[QtWidgets.QDoubleSpinBox] = [(QtWidgets.QDoubleSpinBox.value, QtWidgets.QDoubleSpinBox.setValue)]

def dumpQObjectTree(qobject, level=0):
    """
    A helper function for printing the tree view of QObjects. Qt offers a similar function, but
    it is only available if Qt was a debug build.

    Args:
        qobject[QObject]: The object to dump the tree.
        level[int]: The indent level.
    """

    if level == 0:
        print('+ ' + qobject.objectName() + ' (' + str(type(qobject)) + ')')

    children = qobject.children()
    n = len(children)
    for i in range(n):
        child = children[i]

        if i == 0:
            print('|  '*(level+1))
            prefix = '|  '*(level) + '+--'
        else:
            print('|  '*(level+2))
            prefix = '|  '*(level+1)

        print(prefix + '+ ' + child.objectName() + ' (' + str(type(child)) + ')')
        dumpQObjectTree(child, level+1)

def createIcon(name):
    """
    Create an icon using the "icons" directory.

    Args:
       name[str]: The name of the icon.
    """
    icon_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'icons', name))
    return QtGui.QIcon(icon_path)

def storeWidget(widget, key, **kwargs):
    """
    A function for writing QWidget state into a list of functions to call to restore the state.

    Args:
        widget[QtWidgets.QWidget]: The widget to store settings.
        key(s)[int|str|tuple]: The key to extract from the cache.

    Kwargs:
        debug[bool]: When True debug messages are printed.
    """

    # Handle the optional argument and key, value pairs
    debug = kwargs.pop('debug', message.MOOSE_DEBUG_MODE)

    # Loop over widget storing pairings
    for wtype, methods in WIDGET_SETTINGS_CACHE.iteritems():
        if isinstance(widget, wtype):

            # Debugging information
            msg = ['Storing State: ' + widget.__class__.__name__ + ' ' + str(widget.objectName())]
            msg += ['  key = ' + str(key)]

            # Define storage structure for storing state settings
            state = widget.property('state')
            if not state:
                widget.setProperty('state', dict())

            # Clear current state
            state = widget.property('state')
            state[key] = []

            # Loop through the values to store
            for s in methods:
                attr = s[0]
                state[key].append((s[1], attr(widget)))
                msg += ['    {} ({})'.format(s[1], attr)]

            # Print debug message
            message.mooseDebug('\n'.join(msg), color='GREEN', debug=debug)

            # Update the stored state
            widget.setProperty('state', state)

    # Call store on children
    for child in widget.children():
        storeWidget(child, key, debug=debug)

def loadWidget(widget, key, **kwargs):
    """
    A function for loading GUI state from a storage structure created with storeWidget.

    Args:
        widget[QtWidgets.QWidget]: The widget to load settings.
        key(s)[int|str|tuple]: The key to extract from the cache.

    Kwargs:
        debug[bool]: When True debug messages are printed.
        block[bool]: When True (default) signals are blocked from the widget.
    """

    # Handle optional inputs and convert name to QString
    debug = kwargs.pop('debug', message.MOOSE_DEBUG_MODE)
    block = kwargs.pop('block', True)

    # The stored state dict() of the widget
    state = widget.property('state')
    if not state:
        return

    elif (key not in state) and ('default' in state):
        key = 'default'

    elif key not in state:
        return

    if block:
        widget.blockSignals(True)

    # Debug message
    msg = ['Loading State: ' + widget.__class__.__name__ + ' ' + str(widget.objectName())]
    msg += ['  key = ' + str(key)]

    for func in state[key]:
        msg += [' '*4 + str(func[0]) + '(' + str(func[1]) + ')']
        func[0](widget, func[1])

    # Print debug message
    message.mooseDebug('\n'.join(msg), debug=debug, color='YELLOW')

    if block:
        widget.blockSignals(False)

    # Call load on children
    for child in widget.children():
        loadWidget(child, key, debug=debug, block=block)

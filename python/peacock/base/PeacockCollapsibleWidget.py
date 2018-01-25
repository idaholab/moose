#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5 import QtCore, QtWidgets
from peacock.utils import WidgetUtils

class PeacockCollapsibleWidget(QtWidgets.QGroupBox):
    """
    A group that contains a title with button that allows for collapsing content.

    The contents within layout returned by the "collapsibleLayout" method are collapsed with the
    '+/-' button in the title of the widget.
    """

    ICONSIZE = QtCore.QSize(16, 16)

    def __init__(self, parent=None, title='', collapsed=False, collapsible_layout=QtWidgets.QHBoxLayout, **kwargs):
        super(PeacockCollapsibleWidget, self).__init__(parent)


        # Group Title Widget
        self._title_widget = QtWidgets.QLabel(title)
        self._title_widget.setStyleSheet('font-weight:bold;font-size:12pt')

        # "+/-" Button
        self._collapse_button = QtWidgets.QPushButton()
        self._collapse_button.setIconSize(self.ICONSIZE)
        self._collapse_button.setFixedSize(self._collapse_button.iconSize())
        self._collapse_button.setToolTip("Create python script to reproduce this figure.")
        self._collapse_button.setStyleSheet("QPushButton {border:none}")
        self._collapse_button.clicked.connect(self._callbackHideButton)

        self._main_layout = QtWidgets.QVBoxLayout(self)
        self._main_layout.setContentsMargins(5, 5, 0, 0)

        self._title_layout = QtWidgets.QHBoxLayout()
        self._title_layout.addWidget(self._collapse_button)
        self._title_layout.addWidget(self._title_widget)
        self._title_layout.setAlignment(QtCore.Qt.AlignTop)

        self._collapsible_widget = QtWidgets.QWidget()
        self._collapsible_layout = collapsible_layout(self._collapsible_widget)
        self._collapsible_layout.setContentsMargins(0, 0, 0, 0)
        self._main_layout.addLayout(self._title_layout)
        self._main_layout.addWidget(self._collapsible_widget)
        self._main_layout.setAlignment(QtCore.Qt.AlignTop)

        self._collapsed = None
        self.setCollapsed(collapsed)
        self.setSizePolicy(QtWidgets.QSizePolicy.Minimum, QtWidgets.QSizePolicy.Minimum)

    def collapsibleLayout(self):
        """
        Returns the layout, the contents of this layout will be collapsible.
        """
        return self._collapsible_layout

    def setTitle(self, title):
        """
        Set the title of the widget.
        """
        self._title_widget.setText(title)

    def setCollapsed(self, collapsed):
        """
        Set the collapsed state of the widget.
        """
        self._collapsed = not collapsed
        self._callbackHideButton()

    def isCollapsed(self):
        """
        Return the collapsed state.
        """
        return self._collapsed

    def _callbackHideButton(self):
        """
        Toggles the collapsible content.
        """
        self._collapsed = not self._collapsed
        self._collapsible_widget.setHidden(self._collapsed)
#        PeacockCollapsibleWidget.toggleCollapsed(self._collapsible_layout, self._collapsed)

        name = 'plus.svg' if self._collapsed else 'minus.svg'
        self._collapse_button.setIcon(WidgetUtils.createIcon(name))

    @staticmethod
    def toggleCollapsed(layout, collapsed):
        for i in range(layout.count()):
            item = layout.itemAt(i)
            if item.widget():
                item.widget().setHidden(collapsed)
            if item.layout():
                PeacockCollapsibleWidget.toggleCollapsed(item.layout(), collapsed)

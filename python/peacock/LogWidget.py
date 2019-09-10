#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PyQt5.QtWidgets import QWidget, QTextBrowser
from PyQt5.QtCore import pyqtSlot
from peacock.utils import WidgetUtils
from peacock.base import MooseWidget
from mooseutils import message
from xml.sax.saxutils import escape as escape

class LogWidget(QWidget, MooseWidget):
    """
    A widget that shows the log.

    This ties into mooseutils.message.mooseMessage which will
    emit a signal on mooseutils.message.messageEmitter
    This widget listens to messageEmitter and puts
    the text in the widget.
    Color text is supported.
    """
    def __init__(self, **kwds):
        """
        Constructor.
        """
        super(LogWidget, self).__init__(**kwds)
        self.top_layout = WidgetUtils.addLayout(vertical=True)
        self.setLayout(self.top_layout)
        self.log = QTextBrowser(self)
        self.log.setStyleSheet("QTextBrowser { background: black; color: white; }")
        self.log.setReadOnly(True)
        message.messageEmitter.message.connect(self._write)
        self.button_layout = WidgetUtils.addLayout()
        self.hide_button = WidgetUtils.addButton(self.button_layout, self, "Hide", lambda: self.hide())
        self.clear_button = WidgetUtils.addButton(self.button_layout, self, "Clear", lambda: self.log.clear())
        self.top_layout.addWidget(self.log)
        self.top_layout.addLayout(self.button_layout)
        self.resize(800, 500)

    @pyqtSlot(str, str)
    def _write(self, msg, color):
        """
        This is the slot that will write the message to the widget.

        Inputs:
            msg: The message to write
            color: The color to write the text in.
        """
        if not msg:
            return

        msg = msg.encode('utf-8').decode() # make sure if there are bad characters in the message that we can show them.

        if not color or color == "None":
            color = "white"
        else:
            color = str(color)

        if msg.find("\n") >= 0:
            self.log.insertHtml('<div style="color:%s"><pre><code>%s</code></pre></div><br>' % (color.lower(), escape(msg)))
        else:
            self.log.insertHtml('<div style="color:%s">%s</div><br>' % (color.lower(), escape(msg)))

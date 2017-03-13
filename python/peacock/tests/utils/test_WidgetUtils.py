#!/usr/bin/env python
from peacock.utils import WidgetUtils
from peacock.utils import Testing
from PyQt5.QtCore import QObject

class Tests(Testing.PeacockTester):
    def callback(self):
        pass

    def testCreation(self):
        layout = WidgetUtils.addLayout()
        WidgetUtils.addLineEdit(layout, None, self.callback)
        WidgetUtils.addProgressBar(layout, None, callback=self.callback)
        WidgetUtils.addButton(layout, None, "Button", self.callback)
        WidgetUtils.addLabel(layout, None, "Name")
        WidgetUtils.addCheckbox(layout, None, "name", self.callback)

    def testDump(self):
        objs = [QObject()]
        objs.append(QObject(parent=objs[0]))
        objs.append(QObject(parent=objs[0]))
        objs.append(QObject(parent=objs[1]))
        objs.append(QObject(parent=objs[2]))
        WidgetUtils.dumpQObjectTree(objs[0])

if __name__ == '__main__':
    Testing.run_tests()

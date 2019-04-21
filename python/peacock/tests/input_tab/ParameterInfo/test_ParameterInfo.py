#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from peacock.utils import Testing
from peacock.Input.ParameterInfo import ParameterInfo
try:
    from cStringIO import StringIO
except ImportError:
    from io import StringIO
from PyQt5 import QtWidgets

class Tests(Testing.PeacockTester):
    qapp = QtWidgets.QApplication([])

    def createData(self,
            name,
            default="",
            cpp_type="string",
            basic_type="String",
            description="",
            group_name="",
            required=True,
            options="",
            ):
        return {"name": name,
                "default": default,
                "cpp_type": cpp_type,
                "basic_type": basic_type,
                "description": description,
                "group_name": group_name,
                "required": required,
                "options": options,
                }

    def checkParameter(self,
            p,
            name,
            default="",
            description="",
            value="",
            user_added=False,
            required=False,
            cpp_type="string",
            group_name="Main",
            comments="",
            ):
        self.assertEqual(p.name, name)
        self.assertEqual(p.default, default)
        self.assertEqual(p.description, description)
        self.assertEqual(p.value, value)
        self.assertEqual(p.user_added, user_added)
        self.assertEqual(p.required, required)
        self.assertEqual(p.cpp_type, cpp_type)
        self.assertEqual(p.group_name, group_name)
        self.assertEqual(p.comments, comments)

    def testBasic(self):
        p = ParameterInfo(None, "p0")
        y = self.createData("p1", default="foo", cpp_type="some type", description="description", group_name="group", required=True)
        p.setFromData(y)
        y["default"] = "foo"
        self.checkParameter(p, "p1", value="foo", default="foo", cpp_type="some type", description="description", group_name="group", required=True)

    def testCopy(self):
        p = ParameterInfo(None, "p0")
        p1 = p.copy(None)
        self.assertEqual(p.__dict__, p1.__dict__)

    def testDump(self):
        p = ParameterInfo(None, "p0")
        o = StringIO()
        p.dump(o)
        val = o.getvalue()
        self.assertIn("Name", val)

    def testTypes(self):
        p = ParameterInfo(None, "p0")
        y = self.createData("p1", cpp_type="vector<string>", basic_type="Array", default=None)
        p.setFromData(y)
        self.assertEqual(p.needsQuotes(), True)
        self.assertEqual(p.isVectorType(), True)
        self.assertEqual(p.default, "")
        p.value = "foo"
        self.assertEqual(p.inputFileValue(), "'foo'")

        y = self.createData("p1", cpp_type="bool", basic_type="Boolean", default="0")
        p.setFromData(y)
        self.assertEqual(p.value, "false")
        self.assertEqual(p.default, "false")
        self.assertEqual(p.needsQuotes(), False)
        self.assertEqual(p.isVectorType(), False)
        self.assertEqual(p.inputFileValue(), "false")

        y = self.createData("p1", cpp_type="bool", basic_type="Boolean", default="1")
        p.setFromData(y)
        self.assertEqual(p.value, "true")
        self.assertEqual(p.default, "true")

        y = self.createData("p1", cpp_type="bool")
        p.setFromData(y)
        self.assertEqual(p.value, "false")
        self.assertEqual(p.default, "false")

if __name__ == '__main__':
    Testing.run_tests()

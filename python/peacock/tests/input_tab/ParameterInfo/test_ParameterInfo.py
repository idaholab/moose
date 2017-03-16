#!/usr/bin/env python
from peacock.utils import Testing
from peacock.Input.ParameterInfo import ParameterInfo
import cStringIO

class Tests(Testing.PeacockTester):
    def createData(self,
            name,
            default="",
            cpp_type="string",
            description="",
            group_name="",
            required="Yes",
            options="",
            ):
        return {"name": name,
                "default": default,
                "cpp_type": cpp_type,
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
        y = self.createData("p1", default="foo", cpp_type="some type", description="description", group_name="group", required="Yes")
        p.setFromData(y)
        y["default"] = "foo"
        self.checkParameter(p, "p1", value="foo", default="foo", cpp_type="some type", description="description", group_name="group", required=True)

    def testCopy(self):
        p = ParameterInfo(None, "p0")
        p1 = p.copy(None)
        self.assertEqual(p.__dict__, p1.__dict__)

    def testDump(self):
        p = ParameterInfo(None, "p0")
        o = cStringIO.StringIO()
        p.dump(o)
        val = o.getvalue()
        self.assertIn("Name", val)

    def testTypes(self):
        p = ParameterInfo(None, "p0")
        y = self.createData("p1", cpp_type="vector<string>", default=None)
        p.setFromData(y)
        self.assertEqual(p.needsQuotes(), True)
        self.assertEqual(p.isVectorType(), True)
        self.assertEqual(p.default, "")
        p.value = "foo"
        self.assertEqual(p.inputFileValue(), "'foo'")

        y = self.createData("p1", cpp_type="bool", default="0")
        p.setFromData(y)
        self.assertEqual(p.value, "false")
        self.assertEqual(p.default, "false")
        self.assertEqual(p.needsQuotes(), False)
        self.assertEqual(p.isVectorType(), False)
        self.assertEqual(p.inputFileValue(), "false")

        y = self.createData("p1", cpp_type="bool", default="1")
        p.setFromData(y)
        self.assertEqual(p.value, "true")
        self.assertEqual(p.default, "true")

        y = self.createData("p1", cpp_type="bool")
        p.setFromData(y)
        self.assertEqual(p.value, "false")
        self.assertEqual(p.default, "false")

if __name__ == '__main__':
    Testing.run_tests()

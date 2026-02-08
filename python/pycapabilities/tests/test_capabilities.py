#!/usr/bin/env python3
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test capabilities module."""
import unittest

import pycapabilities

CERTAIN_FAIL = pycapabilities.CheckState.CERTAIN_FAIL
POSSIBLE_FAIL = pycapabilities.CheckState.POSSIBLE_FAIL
POSSIBLE_PASS = pycapabilities.CheckState.POSSIBLE_PASS
CERTAIN_PASS = pycapabilities.CheckState.CERTAIN_PASS

class TestCapabilities(unittest.TestCase):
    """Test capabilities module."""

    def testInit(self):
        """Test pycapabilities.Capabilities.__init__."""
        capabilities = pycapabilities.Capabilities({})
        self.assertIsInstance(capabilities.values, dict)
        self.assertFalse(capabilities.values)

        # Bad arguments
        with self.assertRaisesRegex(
            TypeError, "Capabilities expects a single dict argument"
        ):
            pycapabilities.Capabilities()
        with self.assertRaisesRegex(
            TypeError, "Capabilities expects a single dict argument"
        ):
            pycapabilities.Capabilities("foo")

        # Bad capability dict
        with self.assertRaisesRegex(
            TypeError, "The capability dictionary keys must be strings"
        ):
            pycapabilities.Capabilities({None: None})
        with self.assertRaisesRegex(
            TypeError, "Capability 'name': value is not a dictionary"
        ):
            pycapabilities.Capabilities({"name": None})
        with self.assertRaisesRegex(
            TypeError, "Capability 'name': value 'doc' not a string"
        ):
            pycapabilities.Capabilities({"name": {"doc": None}})
        with self.assertRaisesRegex(KeyError, "missing key 'doc'"):
            pycapabilities.Capabilities({"name": {}})
        with self.assertRaisesRegex(TypeError, "value 'value' of unexpected type"):
            pycapabilities.Capabilities({"name": {"doc": "foo", "value": None}})
        with self.assertRaisesRegex(KeyError, "missing key 'value'"):
            pycapabilities.Capabilities({"name": {"doc": "foo"}})
        with self.assertRaisesRegex(TypeError, "'explicit' value not a bool"):
            pycapabilities.Capabilities(
                {"name": {"doc": "foo", "value": 1, "explicit": None}}
            )
        with self.assertRaisesRegex(
            ValueError, "Capability 'name': 'enumeration' value not iterable"
        ):
            pycapabilities.Capabilities(
                {"name": {"doc": "foo", "value": 1, "enumeration": None}}
            )
        with self.assertRaisesRegex(
            TypeError, "Capability 'name': 'enumeration' value is not a string"
        ):
            pycapabilities.Capabilities(
                {"name": {"doc": "foo", "value": 1, "enumeration": {None: None}}}
            )

        # Catching CapabilityUtil errors
        with self.assertRaisesRegex(
            pycapabilities.CapabilityException,
            r"Capability::setExplicit\(\): Capability 'name' is "
            "bool-valued and cannot be set as explicit",
        ):
            pycapabilities.Capabilities(
                {"name": {"doc": "foo", "value": True, "explicit": True}}
            )
        with self.assertRaisesRegex(
            pycapabilities.CapabilityException,
            r"Capability::setExplicit\(\): Capability 'name' is "
            "bool-valued and cannot be set as explicit",
        ):
            pycapabilities.Capabilities(
                {"name": {"doc": "foo", "value": True, "explicit": True}}
            )

    def testCapabilitiesCheck(self):
        """Test pycapabilities.Capabilities.check with no optional arguments."""
        cap = {
            "bool_true": {"doc": "true value", "value": True},
            "bool_false": {"doc": "false value", "value": False},
            "int": {"doc": "int value", "value": 1, "explicit": False},
            "int_explicit": {"doc": "int explicit value", "value": 1, "explicit": True},
            "version": {"doc": "versioned value", "value": "1.2.3", "explicit": False},
            "string": {"doc": "string value", "value": "string", "explicit": False},
            "string_explicit": {
                "doc": "string explicit value",
                "value": "string",
                "explicit": True,
            },
            "string_enumerated": {
                "doc": "string enumerated value",
                "value": "string",
                "enumeration": ["foo", "string"],
                "explicit": False,
            },
            "string_enumerated_explicit": {
                "doc": "string enumerated explicit value",
                "value": "string",
                "enumeration": ["foo", "string"],
                "explicit": True,
            },
        }

        capabilities = pycapabilities.Capabilities(cap)

        def check(req: str, expect_status: pycapabilities.CheckState):
            status, reason, doc = capabilities.check(req)
            self.assertIsInstance(status, pycapabilities.CheckState)
            self.assertIsInstance(reason, str)
            self.assertIsInstance(doc, str)
            self.assertEqual(expect_status, status)

        # unknown
        check("!unknown", POSSIBLE_PASS)

        # booleans
        check("!bool_true", CERTAIN_FAIL)
        check("!bool_false", CERTAIN_PASS)

        # ints
        check("int=1", CERTAIN_PASS)
        check("int>0", CERTAIN_PASS)
        check("int=2", CERTAIN_FAIL)
        check("int<0", CERTAIN_FAIL)
        check("int", CERTAIN_PASS)

        # strings
        check("string=string", CERTAIN_PASS)
        check("string=foo", CERTAIN_FAIL)
        check("string", CERTAIN_PASS)

        # Explicit capabilities raise
        for name in ["int_explicit", "string_explicit", "string_enumerated_explicit"]:
            with self.assertRaisesRegex(
                pycapabilities.CapabilityException,
                f"Capability statement '{name}': capability '{name}' requires a value "
                "and cannot be used in a boolean expression",
            ):
                capabilities.check(name)
            with self.assertRaisesRegex(
                pycapabilities.CapabilityException,
                f"Capability statement '{name}': capability '{name}' requires a value "
                "and cannot be used in a boolean expression",
            ):
                capabilities.check(f"!{name}")

        # Enumerated capabilities raise
        for name in ["string_enumerated", "string_enumerated_explicit"]:
            with self.assertRaisesRegex(
                pycapabilities.CapabilityException,
                f"Capability statement '{name}=bar': 'bar' invalid for "
                f"capability '{name}'; valid values: foo, string",
            ):
                capabilities.check(f"{name}=bar")

        # Combined requirements
        check("bool_true & !bool_true", CERTAIN_FAIL)
        check("int>0 & (foo | string=string)", CERTAIN_PASS)

    def testCapabilitiesCheckAdd(self):
        """Test pycapabilities.Capabilities.check with add_capabilities."""
        caps = {"orig": {"doc": "doc", "value": "orig"}}
        capabilities = pycapabilities.Capabilities(caps)

        # Normal behavior, capability is added
        self.assertEqual(capabilities.check("added")[0], POSSIBLE_FAIL)
        add = {"added": {"doc": "doc", "value": "added"}}
        self.assertEqual(
            capabilities.check("orig & added", add_capabilities=add)[0],
            CERTAIN_PASS,
        )

        # Bad arguments
        with self.assertRaisesRegex(TypeError, "add_capabilities must be a dict"):
            capabilities.check("unused", add_capabilities="foo")

        # Adding a capability fails (same name)
        add = {"orig": {"doc": "doc", "value": "added"}}
        with self.assertRaises(pycapabilities.CapabilityException):
            capabilities.check("unused", add_capabilities=add)

    def testCapabilitiesCheckNegate(self):
        """Test pycapabilities.Capabilities.check with negate_capabilities."""
        caps = {
            "cap1": {"doc": "doc", "value": "cap1"},
            "cap2": {"doc": "doc", "value": "cap2"},
        }
        capabilities = pycapabilities.Capabilities(caps)

        # One capability is set false
        self.assertEqual(
            capabilities.check("!cap1 & cap2=cap2", negate_capabilities=["cap1"])[0],
            CERTAIN_PASS,
        )
        # Both capabilities are set false
        self.assertEqual(
            capabilities.check("!cap1 & !cap2", negate_capabilities=["cap1", "cap2"])[
                0
            ],
            CERTAIN_PASS,
        )
        # Capability is added
        self.assertEqual(
            capabilities.check(
                "cap1=cap1 & cap2=cap2 & !cap3", negate_capabilities=["cap3"]
            )[0],
            CERTAIN_PASS,
        )
        # One capability is set false, one is added
        self.assertEqual(
            capabilities.check(
                "cap1=cap1 & !cap2 & !cap3", negate_capabilities=["cap2", "cap3"]
            )[0],
            CERTAIN_PASS,
        )

        # Bad arguments
        with self.assertRaisesRegex(
            TypeError, "'negate_capabilities' entry not a string"
        ):
            capabilities.check("unused", negate_capabilities=[1])

    def testCapabilitiesCheckAddNegate(self):
        """Test pycapabilities.Capabilities.check with [add,negate]_capabilities."""
        caps = {
            "cap1": {"doc": "doc", "value": "cap1"},
            "cap2": {"doc": "doc", "value": "cap2"},
        }
        capabilities = pycapabilities.Capabilities(caps)

        add = {
            "cap3": {"doc": "doc", "value": "cap3"},
            "cap4": {"doc": "doc", "value": "cap4"},
        }
        negate = ["cap1", "cap3", "cap5"]

        self.assertEqual(
            capabilities.check(
                "!cap1 & cap2=cap2 & !cap3 & cap4=cap4 & !cap5",
                add_capabilities=add,
                negate_capabilities=negate,
            )[0],
            CERTAIN_PASS,
        )


if __name__ == "__main__":
    unittest.main(module=__name__, verbosity=2)

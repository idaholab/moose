# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test using ad_size in capabilities."""

from unittest import main

from TestHarnessTestCase import TestHarnessTestCase


def buildTestSpecs(**kwargs) -> dict:
    """Build a dummy test spec."""
    return {
        "test": {"type": "RunApp", "input": "unused", "should_execute": False, **kwargs}
    }


class TestHarnessTester(TestHarnessTestCase):
    """Test using ad_size in capabilities."""

    def getADSize(self) -> int:
        """Get the application's AD size."""
        harness = self.runTests(tests=buildTestSpecs(), run=False).harness
        ad_size = harness.options._capabilities["ad_size"][0]
        assert isinstance(ad_size, int)
        return ad_size

    def testSkipLive(self):
        """Test skipping when the ad_size is too small with a live app."""
        ad_size = self.getADSize()
        bad_ad_size = ad_size + 1
        out = self.runTests(
            "--no-color",
            tests=buildTestSpecs(capabilities=f"ad_size>{bad_ad_size}"),
            minimal_capabilities=False,
        ).output
        self.assertRegex(
            out, rf"test\.test[\s.]+\[NEEDS: AD_SIZE>{bad_ad_size}\]\s+SKIP"
        )

    def testGoodLive(self):
        """Test running when the ad_size is large enough with a live app."""
        ad_size = self.getADSize()
        good_ad_size = ad_size - 1
        out = self.runTests(
            "--no-color",
            tests=buildTestSpecs(capabilities=f"ad_size>{good_ad_size}"),
            minimal_capabilities=False,
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")

    def testSkip(self):
        """Test skipping when the ad_size is too small without an app."""
        ad_size = 10
        bad_ad_size = ad_size + 1
        out = self.runTests(
            "--no-color",
            extra_capabilities={"ad_size": [ad_size, "unused doc"]},
            tests=buildTestSpecs(capabilities=f"ad_size>{bad_ad_size}"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(
            out, rf"test\.test[\s.]+\[NEEDS: AD_SIZE>{bad_ad_size}\]\s+SKIP"
        )

    def testGood(self):
        """Test running when the ad_size is large enough without an app."""
        ad_size = 10
        good_ad_size = ad_size - 1
        out = self.runTests(
            "--no-color",
            extra_capabilities={"ad_size": [ad_size, "unused doc"]},
            tests=buildTestSpecs(capabilities=f"ad_size>{good_ad_size}"),
            minimal_capabilities=True,
        ).output
        self.assertRegex(out, r"test\.test[\s.]+OK")


if __name__ == "__main__":
    main()

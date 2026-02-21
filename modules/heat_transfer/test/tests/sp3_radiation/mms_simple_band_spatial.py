#!/usr/bin/env python3

import unittest
import mms
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class TestSimpleBand(unittest.TestCase):
    def test_convergence(self):
        df = run_spatial('simple_band_spatial.i', 4, "--error", console=False)
        fig = mms.ConvergencePlot(xlabel='Average Element Size ($h$)', ylabel='$L_2$ Error')
        fig._axes.set_xscale('log')
        fig._axes.set_yscale('log')

        fig.plot(df, label='CONST', marker='o', markersize=8)
        fig.save('simple_band_mms_spatial.png')

        for key, value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))

        df.to_csv('mms_simple_band_spatial.csv')

if __name__ == "__main__":
    unittest.main()

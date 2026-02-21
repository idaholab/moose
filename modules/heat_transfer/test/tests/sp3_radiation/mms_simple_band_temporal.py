#!/usr/bin/env python3

import unittest
import mms
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_temporal(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../"
        return mms.run_temporal(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../combined/"
        return mms.run_temporal(*args, **kwargs)

class TestSimpleBand(unittest.TestCase):
    def test_convergence(self):
        df = run_temporal('simple_band_temporal.i', 4, console=False, dt=1e-5)
        fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')

        fig.plot(df, label='1st Order (Implicit Euler)', marker='o', markersize=8)
        fig.save('simple_band_mms_temporal.png')

        for key, value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

        df.to_csv('mms_simple_band_temporal.csv')

if __name__ == "__main__":
    unittest.main()


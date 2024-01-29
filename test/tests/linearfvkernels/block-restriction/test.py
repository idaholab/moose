import mms
import unittest
from mooseutils import fuzzyEqual

def run_spatial(*args, **kwargs):
    kwargs['executable'] = "../../../"
    return mms.run_spatial(*args, **kwargs)

class BlockRestrictedDiffusion(unittest.TestCase):
    def test(self):
        df1 = run_spatial('block-restricted-diffusion.i', 5, file_base="block-restricted-diffusion_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('block-restricted-diffusion.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

class BlockRestrictedADR(unittest.TestCase):
    def test(self):
        df1 = run_spatial('block-restricted-adr.i', 5, file_base="block-restricted-adr_csv")

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='l2error',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('block-restricted-adr.png')

        for _,value in fig.label_to_slope.items():
            print("The current slope: ", value)
            self.assertTrue(fuzzyEqual(value, 2., .05))

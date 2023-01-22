import mms
import unittest
from mooseutils import fuzzyEqual

class BrokenDomain(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('diffusion.i', 7, mpi=1)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label='average',
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('broken-domain.png')

        for _,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyEqual(value, 2., .05))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)

import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

class Test1DUpwind(unittest.TestCase):
# class Test1DUpwind():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('test.i', list(range(3,9)), y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class Test1DAverage(unittest.TestCase):
# class Test1DAverage():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('test.i', list(range(3,6)), "interp_method='average'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('average.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class Test1DFreeFlowHLLC(unittest.TestCase):
# class Test1DFreeFlowHLLC():
    def test(self):
        labels = ['L2rho', 'L2rho_u', 'L2rho_et']
        df1 = mms.run_spatial('free-flow-hllc.i', 9, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('free-flow-hllc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class Test1DPorousHLLC(unittest.TestCase):
# class Test1DPorousHLLC():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('porous-hllc.i', 9, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('porous-hllc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class Test1DPorousKTConservative(unittest.TestCase):
# class Test1DPorousKT():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('porous-kt.i', 9, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('porous-kt.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class Test1DPorousKTPrimitive(unittest.TestCase):
# class Test1DPorousKT():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('primitive-interpolated-porous-kt.i', 9, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('primitive-interpolated-porous-kt.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
    # Test1DUpwind().test()
    # Test1DAverage().test()
    # Test1DFreeFlowHLLC().test()
    # Test1DPorousHLLC().test()
    # Test1DPorousKTConservative().test()
    # Test1DPorousKTPrimitive().test()

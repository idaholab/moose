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

class TestBasic1DPorousKTPrimitiveCD(unittest.TestCase):
# class TestBasic1DPorousKTPrimitiveCD():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('basic-primitive-pcnsfv-kt.i', 7, "GlobalParams/limiter='central_difference'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DPorousKTPrimitiveUpwind(unittest.TestCase):
# class TestBasic1DPorousKTPrimitiveUpwind():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('basic-primitive-pcnsfv-kt.i', 9, "GlobalParams/limiter='upwind'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DPorousKTPrimitiveVanLeer(unittest.TestCase):
# class TestBasic1DPorousKTPrimitiveVanLeer():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('basic-primitive-pcnsfv-kt.i', 4, "GlobalParams/limiter='vanLeer'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 2.)
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DPorousKTConservedCD(unittest.TestCase):
# class TestBasic1DPorousKTConservedCD():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('basic-conserved-pcnsfv-kt.i', 7, "GlobalParams/limiter='central_difference'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DPorousKTConservedUpwind(unittest.TestCase):
# class TestBasic1DPorousKTConservedUpwind():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('basic-conserved-pcnsfv-kt.i', 10, "GlobalParams/limiter='upwind'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DPorousKTConservedVanLeer(unittest.TestCase):
# class TestBasic1DPorousKTConservedVanLeer():
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = mms.run_spatial('basic-conserved-pcnsfv-kt.i', 6, "GlobalParams/limiter='vanLeer'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 2.)
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingPorousKTPrimitiveCD(unittest.TestCase):
# class TestBasic1DVaryingPorousKTPrimitiveCD():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-basic-kt-primitive.i', 7, "GlobalParams/limiter='central_difference'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingPorousKTPrimitiveUpwind(unittest.TestCase):
# class TestBasic1DVaryingPorousKTPrimitiveUpwind():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-basic-kt-primitive.i', 9, "GlobalParams/limiter='upwind'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingPorousKTPrimitiveVanLeer(unittest.TestCase):
# class TestBasic1DVaryingPorousKTPrimitiveVanLeer():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-basic-kt-primitive.i', 7, "GlobalParams/limiter='vanLeer'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 1.95)
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingVarPorousKTPrimitiveCD(unittest.TestCase):
# class TestBasic1DVaryingVarPorousKTPrimitiveCD():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-var-basic-kt-primitive.i', 7, "GlobalParams/limiter='central_difference'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-var-basic-kt-primitive-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingVarPorousKTPrimitiveUpwind(unittest.TestCase):
# class TestBasic1DVaryingVarPorousKTPrimitiveUpwind():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-var-basic-kt-primitive.i', 9, "GlobalParams/limiter='upwind'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-var-basic-kt-primitive-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
        # for key,value in fig.label_to_intercept.items():
        #     print("%s intercept, %f" % (key, value))

class TestBasic1DVaryingVarPorousKTPrimitiveVanLeer(unittest.TestCase):
# class TestBasic1DVaryingVarPorousKTPrimitiveVanLeer():
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = mms.run_spatial('varying-eps-var-basic-kt-primitive.i', 8, "GlobalParams/limiter='vanLeer'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-var-basic-kt-primitive-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 1.95)
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
    # TestBasic1DPorousKTPrimitiveCD().test()
    # TestBasic1DPorousKTPrimitiveUpwind().test()
    # TestBasic1DPorousKTPrimitiveVanLeer().test()
    # TestBasic1DPorousKTConservedCD().test()
    # TestBasic1DPorousKTConservedUpwind().test()
    # TestBasic1DPorousKTConservedVanLeer().test()
    # TestBasic1DVaryingPorousKTPrimitiveCD().test()
    # TestBasic1DVaryingPorousKTPrimitiveUpwind().test()
    # TestBasic1DVaryingPorousKTPrimitiveVanLeer().test()
    # TestBasic1DVaryingVarPorousKTPrimitiveCD().test()
    # TestBasic1DVaryingVarPorousKTPrimitiveUpwind().test()
    # TestBasic1DVaryingVarPorousKTPrimitiveVanLeer().test()

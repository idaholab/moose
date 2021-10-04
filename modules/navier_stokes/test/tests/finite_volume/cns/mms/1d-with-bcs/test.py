import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual

def run_spatial(*args, **kwargs):
    try:
        kwargs['executable'] = "../../../../../../"
        return mms.run_spatial(*args, **kwargs)
    except:
        kwargs['executable'] = "../../../../../../../combined/"
        return mms.run_spatial(*args, **kwargs)

class Test1DFreeFlowHLLC(unittest.TestCase):
    def test(self):
        labels = ['L2rho', 'L2rho_u', 'L2rho_et']
        df1 = run_spatial('free-flow-hllc.i', 9, "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('free-flow-hllc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class Test1DPorousHLLC(unittest.TestCase):
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = run_spatial('porous-hllc.i', 9, "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('porous-hllc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class Test1DVaryingEpsPorousHLLC(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_mom_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-hllc.i', 9, "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-hllc.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class TestBasic1DPorousKTPrimitiveCD(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('basic-primitive-pcnsfv-kt.i', 7, "GlobalParams/limiter='central_difference'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestBasic1DPorousKTPrimitiveUpwind(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('basic-primitive-pcnsfv-kt.i', list(range(8,11)), "GlobalParams/limiter='upwind'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class TestBasic1DPorousKTPrimitiveVanLeer(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('basic-primitive-pcnsfv-kt.i', 4, "GlobalParams/limiter='vanLeer'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-primitive-pcnsfv-kt-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 2.)

class TestBasic1DPorousKTConservedCD(unittest.TestCase):
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = run_spatial('basic-conserved-pcnsfv-kt.i', 5, "GlobalParams/limiter='central_difference'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestBasic1DPorousKTConservedUpwind(unittest.TestCase):
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = run_spatial('basic-conserved-pcnsfv-kt.i', 10, "GlobalParams/limiter='upwind'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestBasic1DPorousKTConservedVanLeer(unittest.TestCase):
    def test(self):
        labels = ['L2rho', 'L2rho_ud', 'L2rho_et']
        df1 = run_spatial('basic-conserved-pcnsfv-kt.i', 6, "GlobalParams/limiter='vanLeer'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('basic-conserved-pcnsfv-kt-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 2.)

class TestBasic1DVaryingPorousKNPPrimitiveCD(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 7, "GlobalParams/limiter='central_difference'", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-knp-primitive-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestBasic1DVaryingPorousKNPPrimitiveUpwind(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 9, "GlobalParams/limiter='upwind'", "--error", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-knp-primitive-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class TestBasic1DVaryingPorousKNPPrimitiveVanLeer(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 8, "GlobalParams/limiter='vanLeer'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, slope_precision=1)
        fig.save('varying-eps-basic-knp-primitive-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 1.95)

class TestBasic1DVaryingPorousKTPrimitiveCD(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 7, "GlobalParams/limiter='central_difference'", "GlobalParams/knp_for_omega=false", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestBasic1DVaryingPorousKTPrimitiveUpwind(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 9, "GlobalParams/limiter='upwind'", "GlobalParams/knp_for_omega=false", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class TestBasic1DVaryingPorousKTPrimitiveVanLeer(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_vel_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-primitive.i', 8, "GlobalParams/limiter='vanLeer'", "GlobalParams/knp_for_omega=false", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, slope_precision=1)
        fig.save('varying-eps-basic-kt-primitive-vanLeer.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertGreaterEqual(value, 1.95)

class TestBasic1DVaryingPorousKTMixedCD(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_mom_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-mixed.i', 5, "GlobalParams/limiter='central_difference'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-mixed-cd.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestBasic1DVaryingPorousKTMixedUpwind(unittest.TestCase):
    def test(self):
        labels = ['L2pressure', 'L2sup_mom_x', 'L2T_fluid']
        df1 = run_spatial('varying-eps-basic-kt-mixed.i', 9, "GlobalParams/limiter='upwind'", "--error", y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('varying-eps-basic-kt-mixed-upwind.png')
        for key,value in fig.label_to_slope.items():
            print("%s slope, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class PWCNSFV(unittest.TestCase):
    def test(self):
        df1 = run_spatial('pwcnsfv.i', 8,
                          "--allow-test-objects",
                          "--error",
                          "--error-unused",
                          y_pp=['L2sup_vel_x', 'L2pressure'])

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=['L2sup_vel_x', 'L2pressure'], marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('pwcnsfv.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)

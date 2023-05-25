import mms
import unittest
from mooseutils import fuzzyAbsoluteEqual

class TestOutflow(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('advection-outflow.i', 7, y_pp=['L2u', 'L2v'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u', 'L2v'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('outflow.png')
        for label,value in fig.label_to_slope.items():
            if label == 'L2u':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestOutflowMinMod(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('advection-outflow.i', 7,
                              "GlobalParams/advected_interp_method='min_mod'",
                              "--error-unused",
                              "--error",
                              "--error-deprecated",
                              "--distributed-mesh",
                              y_pp=['L2u', 'L2v'],
                              mpi=2)
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u', 'L2v'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('outflow-min-mod.png')
        for label,value in fig.label_to_slope.items():
            if label == 'L2u':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.5, .05))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class TestExtrapolation(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('advection.i', 7, y_pp=['L2u', 'L2v'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u', 'L2v'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('extrapolation.png')
        for label,value in fig.label_to_slope.items():
            if label == 'L2u':
                self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class UpwindLimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 7, "FVKernels/advection_u/limiter='upwind'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('upwind-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))

class CentralDifferenceLimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 7, "FVKernels/advection_u/limiter='central_difference'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('cd-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class VanLeerLimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 9, "FVKernels/advection_u/limiter='vanLeer'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('vanLeer-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class MinModLimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 9, "FVKernels/advection_u/limiter='min_mod'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('min-mod-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class SOULimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 9, "FVKernels/advection_u/limiter='sou'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('sou-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class QUICKLimiter(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('limited-advection.i', 15, "FVKernels/advection_u/limiter='quick'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('quick-limiter.png')
        for label,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))

class KTLimitedCD(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('kt-limited-advection.i', 11, "FVKernels/advection_u/limiter='central_difference'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('kt-cd-limiter.png')
        for key,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .05))
            print("%s slope, %f" % (key, value))

class KTLimitedUpwind(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('kt-limited-advection.i', 13, "FVKernels/advection_u/limiter='upwind'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('kt-upwind-limiter.png')
        for key,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .05))
            print("%s slope, %f" % (key, value))

class KTLimitedVanLeer(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial('kt-limited-advection.i', 11, "FVKernels/advection_u/limiter='vanLeer'", y_pp=['L2u'])
        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1,
                 label=['L2u'],
                 marker='o',
                 markersize=8,
                 num_fitted_points=3,
                 slope_precision=1)
        fig.save('kt-van-leer-limiter.png')
        for key,value in fig.label_to_slope.items():
            self.assertTrue(fuzzyAbsoluteEqual(value, 2.5, .05))
            print("%s slope, %f" % (key, value))

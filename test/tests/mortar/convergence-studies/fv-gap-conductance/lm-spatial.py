import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual
import sympy

class TestGapConductanceBase(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        super(TestGapConductanceBase, self).__init__(methodName)
        exact_soln = 'x**4 + 2*y**4 +x*y**3'
        f, exact = mms.evaluate('-div(grad(T)) + T', exact_soln, variable='T')

        x,y = sympy.symbols('x y')

        u = eval(str(exact).replace('R.x','x').replace('R.y','y'))

        u_secondary = u.subs(x,1)
        u_primary = u.subs(x,2)
        lm = u_secondary - u_primary

        du_dx = sympy.diff(u, x)

        # flux = n * -grad_u = nx * -du_dx
        # n_secondary = 1
        # n_primary = -1
        flux_secondary = -du_dx.subs(x,1)

        flux_primary = -1 * -du_dx.subs(x,2)

        mms_secondary = flux_secondary - lm
        mms_primary = flux_primary + lm

        self.function_args = [
            "Functions/forcing_function/expression=\'" + mms.fparser(f) + "\'",
            "Functions/exact_soln_primal/expression=\'" + mms.fparser(exact) + "\'",
            "Functions/exact_soln_lambda/expression=\'" + mms.fparser(lm) + "\'",
            "Functions/mms_secondary/expression=\'" + mms.fparser(mms_secondary) + "\'",
            "Functions/mms_primary/expression=\'" + mms.fparser(mms_primary) + "\'"]


class TestGapConductanceFirstOrder(TestGapConductanceBase):
    def __init__(self, methodName='runTest'):
        super(TestGapConductanceFirstOrder, self).__init__(methodName)

    def test(self):
        labels = ['L2lambda', 'L2u']
        df1 = mms.run_spatial('gap-conductance.i', 6, "--error", "--error-unused", "--error-deprecated", *self.function_args, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('first-order.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 1., .1))

class TestGapConductanceSecondOrder(TestGapConductanceBase):
    def __init__(self, methodName='runTest'):
        super(TestGapConductanceSecondOrder, self).__init__(methodName)

    def test(self):
        labels = ['L2lambda', 'L2u']
        df1 = mms.run_spatial('gap-conductance.i', 6, "--error", "--error-unused", "--error-deprecated",
                              "Constraints/mortar/functor_evals_for_primal=true",
                              "Constraints/mortar/ghost_higher_d_neighbors=true",
                              *self.function_args, y_pp=labels)

        fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
        fig.plot(df1, label=labels, marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
        fig.save('second-order.png')
        for key,value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            self.assertTrue(fuzzyAbsoluteEqual(value, 2., .1))


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)

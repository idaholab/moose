import mms
import unittest
import sympy
from sympy import sin, pi, cos
def fuzzyEqual(test_value, true_value, tolerance):
    return abs(test_value - true_value) / abs(true_value) < tolerance
class TestContinuity(unittest.TestCase):
# class TestContinuity(object):
    def __init__(self, methodName='runTest'):
        super(TestContinuity, self).__init__(methodName)
        exact_soln = 'sin(pi*x) * sin(pi*y) * sin(pi*z)'
        f, exact = mms.evaluate('-div(grad(T)) + T', exact_soln, variable='T')
        x,y,z = sympy.symbols('x y z')
        u = eval(str(exact).replace('R.x','x').replace('R.y','y').replace('R.z','z'))
        du_dx = sympy.diff(u, x)
        lm = du_dx
        self.function_args = [
            'Functions/forcing_function/value="' + mms.fparser(f) + '"',
            'Functions/exact_soln_primal/value="' + mms.fparser(exact) + '"',
            'Functions/exact_soln_lambda/value="' + mms.fparser(lm) + '"']
        self.gold_values = {
            'p1p0-u_0' : 1.9377283124612794,
            'p1p0-lm_0' : 1.077753324242711,
            'p1p0-u_0.4' : 1.9333420708370546,
            'p1p0-lm_0.4' : 1.0119801248538074,
            'p1p1-u_0' : 1.9863327490325773,
            'p1p1-lm_0' : 1.9981889938191515,
            'p1p1-u_0.4' : 1.9700137771303148,
            'p1p1-lm_0.4' : 1.6806101907253725,
            'p1p1dual-u_0' : 1.9917618834617183,
            'p1p1dual-lm_0' : 1.5211652338088115,
            'p1p1dual-u_0.4' : 1.9725965200427749,
            'p1p1dual-lm_0.4' : 1.5755342813487287
        }

        self.tolerance = 1e-3
    def do_plot(self, input_file, num_refinements, cli_args, figure, label, mpi=1):
        cli_args = " ".join(cli_args + self.function_args)
        df = mms.run_spatial(input_file,
                             num_refinements,
                             cli_args,
                             x_pp='h',
                             y_pp=['L2u', 'L2lambda'],
                             mpi=mpi)
        figure.plot(df,
                    label=['u_' + label, 'lm_' + label],
                    num_fitted_points=3,
                    slope_precision=1,
                    marker='o')
    def secondary_and_primary_plots(self,
                               fine,
                               num_refinements,
                               additional_cli_args,
                               name,
                               plots_to_do=None,
                               mpi=1):
        fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
        second_order = False
        for cli_arg in additional_cli_args:
            if "SECOND" in cli_arg:
                second_order = True
        if second_order:
            deltas = [0, 0.1]
        else:
            deltas = [0, 0.4]
        for delta in deltas:
            self.do_plot('continuity.i',
                         num_refinements,
                         ["Constraints/mortar/delta="+str(delta)] + additional_cli_args,
                         fig,
                         str(delta),
                         mpi)
        fig.set_title(name)
        fig.save(name+'.png')
        return fig
    def run_geometric_discretization(self, geom_disc, fine_values, num_refinements, mpi=1):
        if geom_disc == "p1p0":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=MONOMIAL",
                        "Variables/lambda/order=CONSTANT",
                        "Variables/lambda/use_dual=false",
                        "Mesh/left_block/elem_type=HEX8",
                        "Mesh/right_block/elem_type=HEX8"]
        elif geom_disc == "p1p1":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=false",
                        "Mesh/left_block/elem_type=HEX8",
                        "Mesh/right_block/elem_type=HEX8"]
        elif geom_disc == "p1p1dual":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=true",
                        "Mesh/left_block/elem_type=HEX8",
                        "Mesh/right_block/elem_type=HEX8"]
        elif geom_disc == "p2p1":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=false",
                        "Mesh/left_block/elem_type=HEX8",
                        "Mesh/right_block/elem_type=HEX8"]
        name_to_slope = {}
        for fine_value in fine_values:
            name = geom_disc
            fig = self.secondary_and_primary_plots(fine_value,
                                              num_refinements,
                                              cli_args,
                                              name,
                                              mpi=mpi)
            for key, value in fig.label_to_slope.items():
                name_to_slope.update({"-".join([name, key]) : value})
        for key, test_value in name_to_slope.items():
            # print("{} : {}".format(key, test_value))
            gold_value = self.gold_values.get(key)
            self.assertIsNotNone(gold_value)
            self.assertTrue(fuzzyEqual(test_value, gold_value, self.tolerance))
    def do_it_all(self, fine_values, num_refinements, mpi=1):
         self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)
         self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)
         self.run_geometric_discretization("p1p1dual", fine_values, num_refinements, mpi)
         self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)
         for key, test_value in self.gold_values.items():
             print("{} : {}".format(key, test_value))

class P1P0(TestContinuity):
    def testP1P0(self, fine_values=[2], num_refinements=4, mpi=1):
        self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)
class P1P1(TestContinuity):
    def testP1P1(self, fine_values=[2], num_refinements=4, mpi=1):
        self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)
class P1P1dual(TestContinuity):
    def testP1P1(self, fine_values=[2], num_refinements=4, mpi=1):
        self.run_geometric_discretization("p1p1dual", fine_values, num_refinements, mpi)
class P2P1(TestContinuity):
    def testP2P1(self, fine_values=[2], num_refinements=4, mpi=0):
        self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)
# instance = TestContinuity()
# instance.do_it_all([3], 8, 32)
# # instance.secondary_and_primary_plots(2,
# #                                 8,
# #                                 "Mesh/second_order=false " + \
# #                                 "Variables/T/order=FIRST " + \
# #                                 "Variables/lambda/family=MONOMIAL " + \
# #                                 "Variables/lambda/order=CONSTANT",
# #                                 "p1p0-asymptotic-same",
# #                                 ['same'],
# #                                 32)

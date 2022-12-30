#!/usr/bin/env python3

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
            'Functions/forcing_function/expression="' + mms.fparser(f) + '"',
            'Functions/exact_soln_primal/expression="' + mms.fparser(exact) + '"',
            'Functions/exact_soln_lambda/expression="' + mms.fparser(lm) + '"']
        self.gold_values = {
            'p1p0_hex-u_0.1' : 1.9305390060119176,
            'p1p0_hex-lm_0.1' : 1.0272242755193284,
            'p1p0_hex_non-u_0.1' : 2.0694003778178534,
            'p1p0_hex_non-lm_0.1' : 1.0719372819043176,
            'p1p1_hex-u_0.1' : 1.90978686865625,
            'p1p1_hex-lm_0.1' : 3.0413581999654924,
            'p1p1_hex_non-u_0.1' : 1.9421386178496052,
            'p1p1_hex_non-lm_0.1' : 2.001745263935625,
            'p1p1dual_hex-u_0.1' : 1.8956170085743176,
            'p1p1dual_hex-lm_0.1' : 1.4802316459242533,
            'p1p1dual_hex_non-u_0.1' : 1.9009463364413586,
            'p1p1dual_hex_non-lm_0.1' : 1.4766123810566583,
            'p2p0_hex-u_0.1' : 3.044201176042964,
            'p2p0_hex-lm_0.1' : 1.019296943618726,
            'p2p0_hex_non-u_0.1' : 2.9750765225049074,
            'p2p0_hex_non-lm_0.1' : 1.019447305041749,
            'p2p1_hex-u_0.1' : 2.9717861674588906,
            'p2p1_hex-lm_0.1' : 2.032637929038658,
            'p2p1_hex_non-u_0.1' : 2.9413624940907344,
            'p2p1_hex_non-lm_0.1' : 2.0321190357379546,
            'p2p2_hex-u_0.1' : 2.939071712671479,
            'p2p2_hex-lm_0.1' : 3.0514171179331036,
            'p2p2_hex_non-u_0.1' : 2.90086020890949,
            'p2p2_hex_non-lm_0.1' : 2.130510552719881
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
                               num_refinements,
                               additional_cli_args,
                               name,
                               plots_to_do=None,
                               mpi=1):
        fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
        has_tets = True
        for cli_arg in additional_cli_args:
            if "hex" in cli_arg:
                has_tets = False
        if has_tets:
            deltas = [0.1]
        else:
            deltas = [0.1]
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
    def run_geometric_discretization(self, geom_disc, elem_types, num_refinements, mpi=1):
        if geom_disc == "p1p0":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=MONOMIAL",
                        "Variables/lambda/order=CONSTANT",
                        "Variables/lambda/use_dual=false",
                        "Mesh/file/place_holder"]
        elif geom_disc == "p1p1":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=false",
                        "Mesh/file/place_holder"]
        elif geom_disc == "p1p1dual":
            cli_args = ["Variables/T/order=FIRST",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=true",
                        "Mesh/file/place_holder"]
        elif geom_disc == "p2p0":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=MONOMIAL",
                        "Variables/lambda/order=CONSTANT",
                        "Variables/lambda/use_dual=false",
                        "Mesh/file/place_holder"]
        elif geom_disc == "p2p1":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST",
                        "Variables/lambda/use_dual=false",
                        "Mesh/file/place_holder"]
        elif geom_disc == "p2p2":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=SECOND",
                        "Variables/lambda/use_dual=false",
                        "Mesh/file/place_holder"]
        name_to_slope = {}
        for elem_type in elem_types:
            name = geom_disc + "_" + elem_type
            cli_args[-1] = "Mesh/file/file=" + elem_type + "_mesh.e"
            fig = self.secondary_and_primary_plots(num_refinements,
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
    def do_it_all(self, elem_types, num_refinements, mpi=1):
         self.run_geometric_discretization("p1p0", elem_types, num_refinements, mpi)
         self.run_geometric_discretization("p1p1", elem_types, num_refinements, mpi)
         self.run_geometric_discretization("p1p1dual", elem_types, num_refinements, mpi)
         self.run_geometric_discretization("p2p0", elem_types, num_refinements, mpi)
         self.run_geometric_discretization("p2p1", elem_types, num_refinements, mpi)
         self.run_geometric_discretization("p2p2", elem_types, num_refinements, mpi)
         for key, test_value in self.gold_values.items():
             print("{} : {}".format(key, test_value))

class P1P0(TestContinuity):
    def testP1P0(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p1p0", elem_types, num_refinements, mpi)
class P1P1(TestContinuity):
    def testP1P1(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p1p1", elem_types, num_refinements, mpi)
class P1P1dual(TestContinuity):
    def testP1P1(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p1p1dual", elem_types, num_refinements, mpi)
class P2P0(TestContinuity):
    def testP2P0(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p0", elem_types, num_refinements, mpi)
class P2P1(TestContinuity):
    def testP2P1(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p1", elem_types, num_refinements, mpi)
class P2P2(TestContinuity):
    def testP2P2(self, elem_types=["hex", "hex_non"], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p2", elem_types, num_refinements, mpi)

if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)

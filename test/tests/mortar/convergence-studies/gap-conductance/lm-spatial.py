import mms
import unittest
import sympy

def fuzzyEqual(test_value, true_value, tolerance):
    return abs(test_value - true_value) / abs(true_value) < tolerance

class TestGapConductance(unittest.TestCase):
    def __init__(self, methodName='runTest'):
        super(TestGapConductance, self).__init__(methodName)

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

        self.gold_values = {
            'p2p2-2-to-2-u_same' : 2.9822564691564524,
            'p2p2-2-to-2-lm_same' : 3.001026070690089,
            'p2p2-3-to-2-u_coarse_primary' : 2.9830362913692774,
            'p2p2-3-to-2-lm_coarse_primary' : 2.9165033066603714,
            'p2p2-3-to-2-u_coarse_secondary' : 2.974976328637989,
            'p2p2-3-to-2-lm_coarse_secondary' : 3.210239428561721,
            'p2p2-4-to-2-u_coarse_primary' : 2.982552524913371,
            'p2p2-4-to-2-lm_coarse_primary' : 2.9082806315999687,
            'p2p2-4-to-2-u_coarse_secondary' : 2.967163866045459,
            'p2p2-4-to-2-lm_coarse_secondary' : 3.2244149963833273,
            'p2p2-5-to-2-u_coarse_primary' : 2.9823936432051203,
            'p2p2-5-to-2-lm_coarse_primary' : 2.9052727051947764,
            'p2p2-5-to-2-u_coarse_secondary' : 2.964889659684415,
            'p2p2-5-to-2-lm_coarse_secondary' : 3.2290095723998236,
            'p2p1-2-to-2-u_same' : 2.9823879971978347,
            'p2p1-2-to-2-lm_same' : 2.027908692116612,
            'p2p1-3-to-2-u_coarse_primary' : 2.9830517599713926,
            'p2p1-3-to-2-lm_coarse_primary' : 2.289722443118018,
            'p2p1-3-to-2-u_coarse_secondary' : 2.9748053887963235,
            'p2p1-3-to-2-lm_coarse_secondary' : 2.039540390360873,
            'p2p1-4-to-2-u_coarse_primary' : 2.982541009917219,
            'p2p1-4-to-2-lm_coarse_primary' : 2.4516029901427503,
            'p2p1-4-to-2-u_coarse_secondary' : 2.9666727450503494,
            'p2p1-4-to-2-lm_coarse_secondary' : 2.043610660107857,
            'p2p1-5-to-2-u_coarse_primary' : 2.982377963288517,
            'p2p1-5-to-2-lm_coarse_primary' : 2.75101171627186,
            'p2p1-5-to-2-u_coarse_secondary' : 2.9643261771297698,
            'p2p1-5-to-2-lm_coarse_secondary' : 2.0449872997477305,
            'p2p0-2-to-2-u_same' : 2.865934548905254,
            'p2p0-2-to-2-lm_same' : 0.9544384151198305,
            'p2p0-3-to-2-u_coarse_primary' : 2.9513681749051264,
            'p2p0-3-to-2-lm_coarse_primary' : 0.9871747739768003,
            'p2p0-3-to-2-u_coarse_secondary' : 2.62873082898225,
            'p2p0-3-to-2-lm_coarse_secondary' : 0.9551048423731033,
            'p2p0-4-to-2-u_coarse_primary' : 2.9719537374138345,
            'p2p0-4-to-2-lm_coarse_primary' : 1.0154487382629993,
            'p2p0-4-to-2-u_coarse_secondary' : 2.5504738003975547,
            'p2p0-4-to-2-lm_coarse_secondary' : 0.9542477401583049,
            'p2p0-5-to-2-u_coarse_primary' : 2.9778820927330556,
            'p2p0-5-to-2-lm_coarse_primary' : 1.0198046269451437,
            'p2p0-5-to-2-u_coarse_secondary' : 2.5332102827019036,
            'p2p0-5-to-2-lm_coarse_secondary' : 0.9543603794667151,
            'p1p1-2-to-2-u_same' : 1.9859815251171462,
            'p1p1-2-to-2-lm_same' : 2.011595941881354,
            'p1p1-3-to-2-u_coarse_primary' : 1.9885251789927607,
            'p1p1-3-to-2-lm_coarse_primary' : 2.0158955961283245,
            'p1p1-3-to-2-u_coarse_secondary' : 1.9792698106931648,
            'p1p1-3-to-2-lm_coarse_secondary' : 1.9981271512915104,
            'p1p1-4-to-2-u_coarse_primary' : 1.9885475293380597,
            'p1p1-4-to-2-lm_coarse_primary' : 1.9977186875839374,
            'p1p1-4-to-2-u_coarse_secondary' : 1.9580169542840375,
            'p1p1-4-to-2-lm_coarse_secondary' : 1.9403777939060056,
            'p1p1-5-to-2-u_coarse_primary' : 1.988512993102887,
            'p1p1-5-to-2-lm_coarse_primary' : 2.0006441170884512,
            'p1p1-5-to-2-u_coarse_secondary' : 1.9388833409209203,
            'p1p1-5-to-2-lm_coarse_secondary' : 1.8767785071182106,
            'p1p0-2-to-2-u_same' : 1.9860344770137297,
            'p1p0-2-to-2-lm_same' : 1.516294286226134,
            'p1p0-3-to-2-u_coarse_primary' : 1.9883909312827506,
            'p1p0-3-to-2-lm_coarse_primary' : 1.7057016469391288,
            'p1p0-3-to-2-u_coarse_secondary' : 1.979352405490139,
            'p1p0-3-to-2-lm_coarse_secondary' : 1.1510942533114046,
            'p1p0-4-to-2-u_coarse_primary' : 1.9884810274602154,
            'p1p0-4-to-2-lm_coarse_primary' : 1.8106476095984056,
            'p1p0-4-to-2-u_coarse_secondary' : 1.95784294017922,
            'p1p0-4-to-2-lm_coarse_secondary' : 1.0201644481503622,
            'p1p0-5-to-2-u_coarse_primary' : 1.9884579467005514,
            'p1p0-5-to-2-lm_coarse_primary' : 1.8694171519114244,
            'p1p0-5-to-2-u_coarse_secondary' : 1.937763546002798,
            'p1p0-5-to-2-lm_coarse_secondary' : 0.9892939793245561
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

        if not plots_to_do:
            if (fine == 2):
                plots_to_do = ['same']
            else:
                plots_to_do = ['coarse_secondary', 'coarse_primary']

        fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')

        if 'coarse_primary' in plots_to_do:
            self.do_plot('gap-conductance.i',
                         num_refinements,
                         ["Mesh/left_block/ny="+str(fine),
                          "Mesh/left_block/nx="+str(fine)] + \
                         additional_cli_args,
                         fig,
                         "coarse_primary",
                         mpi)
        if 'coarse_secondary' in plots_to_do:
            self.do_plot('gap-conductance.i',
                         num_refinements,
                         ["Mesh/right_block/ny="+str(fine),
                          "Mesh/right_block/nx="+str(fine)] + \
                         additional_cli_args,
                         fig,
                         "coarse_secondary",
                         mpi)
        if 'same' in plots_to_do:
            self.do_plot('gap-conductance.i',
                         num_refinements,
                         additional_cli_args,
                         fig,
                         "same",
                         mpi)

        fig.set_title(name)
        fig.save(name+'.png')

        return fig

    def run_geometric_discretization(self, geom_disc, fine_values, num_refinements, mpi=1):
        if geom_disc == "p2p2":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=SECOND"]

        elif geom_disc == "p2p1":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST"]

        elif geom_disc == "p2p0":
            cli_args = ["Mesh/second_order=true",
                        "Variables/T/order=SECOND",
                        "Variables/lambda/family=MONOMIAL",
                        "Variables/lambda/order=CONSTANT"]

        elif geom_disc == "p1p1":
            cli_args = ["Mesh/second_order=false",
                        "Variables/T/order=FIRST",
                        "Variables/lambda/family=LAGRANGE",
                        "Variables/lambda/order=FIRST"]

        elif geom_disc == "p1p0":
            cli_args = ["Mesh/second_order=false",
                        "Variables/T/order=FIRST",
                        "Variables/lambda/family=MONOMIAL",
                        "Variables/lambda/order=CONSTANT"]

        name_to_slope = {}
        for fine_value in fine_values:
            name = geom_disc + "-" + str(fine_value) + "-to-2"
            fig = self.secondary_and_primary_plots(fine_value,
                                              num_refinements,
                                              cli_args,
                                              name,
                                              mpi=mpi)
            for key, value in fig.label_to_slope.items():
                name_to_slope.update({"-".join([name, key]) : value})

        for key, test_value in name_to_slope.items():
            gold_value = self.gold_values.get(key)
            self.assertIsNotNone(gold_value)
            self.assertTrue(fuzzyEqual(test_value, gold_value, self.tolerance))

    def do_it_all(self, fine_values, num_refinements, mpi=1):
        self.run_geometric_discretization("p2p2", fine_values, num_refinements, mpi)
        self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)
        self.run_geometric_discretization("p2p0", fine_values, num_refinements, mpi)
        self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)
        self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)

class P2P2(TestGapConductance):
    def testP2P2(self, fine_values=[2,3,4,5], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p2", fine_values, num_refinements, mpi)

class P2P1(TestGapConductance):
    def testP2P1(self, fine_values=[2,3,4,5], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)

class P2P0(TestGapConductance):
    def testP2P0(self, fine_values=[2,3,4,5], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p2p0", fine_values, num_refinements, mpi)

class P1P1(TestGapConductance):
    def testP1P1(self, fine_values=[2,3,4,5], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)

class P1P0(TestGapConductance):
    def testP1P0(self, fine_values=[2,3,4,5], num_refinements=3, mpi=1):
        self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)


if __name__ == '__main__':
    unittest.main(__name__, verbosity=2)


# instance = TestGapConductance()
# instance.do_it([2,3,4,5], 3, 4)
# instance.secondary_and_primary_plots(2,
#                                 8,
#                                 "Mesh/second_order=false " + \
#                                 "Variables/T/order=FIRST " + \
#                                 "Variables/lambda/family=MONOMIAL " + \
#                                 "Variables/lambda/order=CONSTANT",
#                                 "p1p0-asymptotic-same",
#                                 ['same'],
#                                 32)

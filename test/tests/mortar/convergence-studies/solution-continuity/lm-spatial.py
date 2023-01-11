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

        exact_soln = 'sin(pi*x/2) + cos(pi*y/2) + cos(pi*x/2)*sin(pi*y/2)'
        f, exact = mms.evaluate('-div(grad(T)) + T', exact_soln, variable='T')

        x,y = sympy.symbols('x y')

        u = eval(str(exact).replace('R.x','x').replace('R.y','y'))

        du_dx = sympy.diff(u, x)

        # lambda = normals_secondary * diff_secondary * grad_u_secondary
        lm = 1 * 1 * du_dx

        self.function_args = [
            'Functions/forcing_function/expression="' + mms.fparser(f) + '"',
            'Functions/exact_soln_primal/expression="' + mms.fparser(exact) + '"',
            'Functions/exact_soln_lambda/expression="' + mms.fparser(lm) + '"']

        # self.gold_values = {}
        self.gold_values = {
            'p2p2-2-to-2-u_same_0' : 2.998634817909304,
            'p2p2-2-to-2-lm_same_0' : 3.1044084969914914,
            'p2p2-2-to-2-u_same_0.1' : 2.9921663752381713,
            'p2p2-2-to-2-lm_same_0.1' : 3.0013385403667137,
            'p2p2-5-to-2-u_coarse_primary_0' : 2.9997890026770926,
            'p2p2-5-to-2-lm_coarse_primary_0' : 2.0108573719845966,
            'p2p2-5-to-2-u_coarse_secondary_0' : 2.9950460951663347,
            'p2p2-5-to-2-lm_coarse_secondary_0' : 2.5166142835950485,
            'p2p2-5-to-2-u_coarse_primary_0.1' : 2.9939611035980707,
            'p2p2-5-to-2-lm_coarse_primary_0.1' : 1.9917147987299368,
            'p2p2-5-to-2-u_coarse_secondary_0.1' : 2.984744760123336,
            'p2p2-5-to-2-lm_coarse_secondary_0.1' : 2.5199014186060733,
            'p2p1-2-to-2-u_same_0' : 2.999509642372042,
            'p2p1-2-to-2-lm_same_0' : 2.0029054061613767,
            'p2p1-2-to-2-u_same_0.1' : 2.9927517172776374,
            'p2p1-2-to-2-lm_same_0.1' : 2.0028862537146783,
            'p2p1-5-to-2-u_coarse_primary_0' : 2.999609849486302,
            'p2p1-5-to-2-lm_coarse_primary_0' : 1.9829370834328883,
            'p2p1-5-to-2-u_coarse_secondary_0' : 2.999287138706176,
            'p2p1-5-to-2-lm_coarse_secondary_0' : 2.0033149322329113,
            'p2p1-5-to-2-u_coarse_primary_0.1' : 2.993889703627252,
            'p2p1-5-to-2-lm_coarse_primary_0.1' : 1.9883973118675258,
            'p2p1-5-to-2-u_coarse_secondary_0.1' : 2.9883615175154254,
            'p2p1-5-to-2-lm_coarse_secondary_0.1' : 2.003296665788549,
            'p2p0-2-to-2-u_same_0' : 3.0040747968605213,
            'p2p0-2-to-2-lm_same_0' : 1.0084238164344301,
            'p2p0-2-to-2-u_same_0.1' : 2.9738396134277854,
            'p2p0-2-to-2-lm_same_0.1' : 1.006655545137675,
            'p2p0-5-to-2-u_coarse_primary_0' : 2.999005720972765,
            'p2p0-5-to-2-lm_coarse_primary_0' : 1.006063662172058,
            'p2p0-5-to-2-u_coarse_secondary_0' : 2.725783281979175,
            'p2p0-5-to-2-lm_coarse_secondary_0' : 1.00795578413039,
            'p2p0-5-to-2-u_coarse_primary_0.1' : 2.990919278609015,
            'p2p0-5-to-2-lm_coarse_primary_0.1' : 1.0027757510416793,
            'p2p0-5-to-2-u_coarse_secondary_0.1' : 2.7410956848417696,
            'p2p0-5-to-2-lm_coarse_secondary_0.1' : 1.0068819161885127,
            'p1p1-2-to-2-u_same_0' : 1.9976222558440346,
            'p1p1-2-to-2-lm_same_0' : 2.011332498808176,
            'p1p1-2-to-2-u_same_0.4' : 2.192010143004469,
            'p1p1-2-to-2-lm_same_0.4' : 1.9847827143779497,
            'p1p1-5-to-2-u_coarse_primary_0' : 1.9992491291760002,
            'p1p1-5-to-2-lm_coarse_primary_0' : 0.9986030842919351,
            'p1p1-5-to-2-u_coarse_secondary_0' : 1.999544699253089,
            'p1p1-5-to-2-lm_coarse_secondary_0' : 1.971155450737158,
            'p1p1-5-to-2-u_coarse_primary_0.4' : 2.1095731087882266,
            'p1p1-5-to-2-lm_coarse_primary_0.4' : 0.9807155521591079,
            'p1p1-5-to-2-u_coarse_secondary_0.4' : 2.188809611484894,
            'p1p1-5-to-2-lm_coarse_secondary_0.4' : 1.8873233690453122,
            'p1p0-2-to-2-u_same_0' : 1.997711692307121,
            'p1p0-2-to-2-lm_same_0' : 1.001565938454369,
            'p1p0-2-to-2-u_same_0.4' : 2.1916263122904245,
            'p1p0-2-to-2-lm_same_0.4' : 1.0033834748271102,
            'p1p0-5-to-2-u_coarse_primary_0' : 2.0034278183745324,
            'p1p0-5-to-2-lm_coarse_primary_0' : 1.0440256530245198,
            'p1p0-5-to-2-u_coarse_secondary_0' : 2.001068175514392,
            'p1p0-5-to-2-lm_coarse_secondary_0' : 1.0116336217198654,
            'p1p0-5-to-2-u_coarse_primary_0.4' : 2.109020679157957,
            'p1p0-5-to-2-lm_coarse_primary_0.4' : 0.9979231357062565,
            'p1p0-5-to-2-u_coarse_secondary_0.4' : 2.1903147362107327,
            'p1p0-5-to-2-lm_coarse_secondary_0.4' : 1.0053519515138674
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

        second_order = False
        for cli_arg in additional_cli_args:
            if "SECOND" in cli_arg:
                second_order = True

        if second_order:
            deltas = [0, 0.1]
        else:
            deltas = [0, 0.4]

        for delta in deltas:
            if 'coarse_primary' in plots_to_do:
                self.do_plot('continuity.i',
                             num_refinements,
                             ["Mesh/left_block/ny="+str(fine),
                              "Mesh/left_block/nx="+str(fine),
                              "Constraints/mortar/delta="+str(delta)] + \
                             additional_cli_args,
                             fig,
                             "coarse_primary_"+str(delta),
                             mpi)
            if 'coarse_secondary' in plots_to_do:
                self.do_plot('continuity.i',
                             num_refinements,
                             ["Mesh/right_block/ny="+str(fine),
                              "Mesh/right_block/nx="+str(fine),
                              "Constraints/mortar/delta="+str(delta)] + \
                             additional_cli_args,
                             fig,
                             "coarse_secondary_"+str(delta),
                             mpi)
            if 'same' in plots_to_do:
                self.do_plot('continuity.i',
                             num_refinements,
                             ["Constraints/mortar/delta="+str(delta)] + additional_cli_args,
                             fig,
                             "same_"+str(delta),
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
                # self.gold_values.update({"-".join([name, key]) : value})

        for key, test_value in name_to_slope.items():
            gold_value = self.gold_values.get(key)
            self.assertIsNotNone(gold_value)
            self.assertTrue(fuzzyEqual(test_value, gold_value, self.tolerance))

    def do_it_all(self, fine_values, num_refinements, mpi=1):
        # self.run_geometric_discretization("p2p2", fine_values, num_refinements, mpi)
        # self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)
        self.run_geometric_discretization("p2p0", fine_values, num_refinements, mpi)
        # self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)
        # self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)

        # for key, test_value in self.gold_values.items():
        #     print("{} : {}".format(key, test_value))

class P2P2(TestContinuity):
    def testP2P2(self, fine_values=[2,5], num_refinements=5, mpi=1):
        self.run_geometric_discretization("p2p2", fine_values, num_refinements, mpi)

class P2P1(TestContinuity):
    def testP2P1(self, fine_values=[2,5], num_refinements=5, mpi=1):
        self.run_geometric_discretization("p2p1", fine_values, num_refinements, mpi)

class P2P0(TestContinuity):
    def testP2P0(self, fine_values=[2,5], num_refinements=5, mpi=1):
        self.run_geometric_discretization("p2p0", fine_values, num_refinements, mpi)

class P1P1(TestContinuity):
    def testP1P1(self, fine_values=[2,5], num_refinements=5, mpi=1):
        self.run_geometric_discretization("p1p1", fine_values, num_refinements, mpi)

class P1P0(TestContinuity):
    def testP1P0(self, fine_values=[2,5], num_refinements=5, mpi=1):
        self.run_geometric_discretization("p1p0", fine_values, num_refinements, mpi)


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

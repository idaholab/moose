import mms
import unittest
from mooseutils import fuzzyEqual, fuzzyAbsoluteEqual


class TestChannel(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial(
            "mms-channel.i",
            6,
            "--error",
            "--error-unused",
            y_pp=["L2u", "L2v", "L2p"],
            mpi=4,
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label=["L2u", "L2v", "L2p"],
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("Channel.png")
        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key == "L2p":
                self.assertTrue(fuzzyAbsoluteEqual(value, 1.0, 0.1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.1))


class TestChannelSecond(unittest.TestCase):
    def test(self):
        df1 = mms.run_spatial(
            "mms-channel.i",
            5,
            "--error",
            "--error-unused",
            "k=SECOND",
            "k_minus_one=FIRST",
            "pressure_family=L2_HIERARCHIC",
            y_pp=["L2u", "L2v", "L2p"],
            mpi=4,
        )

        fig = mms.ConvergencePlot(xlabel="Element Size ($h$)", ylabel="$L_2$ Error")
        fig.plot(
            df1,
            label=["L2u", "L2v", "L2p"],
            marker="o",
            markersize=8,
            num_fitted_points=3,
            slope_precision=1,
        )
        fig.save("second-Channel.png")
        for key, value in fig.label_to_slope.items():
            print("%s, %f" % (key, value))
            if key == "L2p":
                self.assertTrue(fuzzyAbsoluteEqual(value, 2.0, 0.1))
            else:
                self.assertTrue(fuzzyAbsoluteEqual(value, 3.0, 0.1))


if __name__ == "__main__":
    unittest.main(__name__, verbosity=2)

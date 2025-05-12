"""Contains test cases for CoupledVectorValueOld function"""

import unittest

import numpy as np

class TestMonomialCoupledVectorValueOld(unittest.TestCase):
    """Test for constant monomials."""
    def read_time_step(self, tstep: int, old: bool) -> np.array:
        """Set reader time step and read data

        Parameters
        ----------
        tstep : int
            Time step for the old variable
        old : bool
            Whether to read var or old_var

        Returns
        -------
        np.array
            Data from csv file
        """

        var = 'old_var' if old else 'var'
        n = tstep if old else tstep - 1

        file = f'coupled_old_vector_out_{var}_{str(n).zfill(4)}.csv'

        return np.loadtxt(file,
                          delimiter=',',
                          skiprows=1)

    def test_old_vector(self):
        """Compares old value at present step with value at previous step"""
        for i in range(2,10):
            data_old = self.read_time_step(i, True)
            data_new = self.read_time_step(i, False)

            assert np.allclose(data_old, data_new)



class TestNodalCoupledVectorValueOld(unittest.TestCase):
    """Test for nodal variables."""
    def read_time_step(self, tstep: int, old: bool) -> np.array:
        """Set reader time step and read data

        Parameters
        ----------
        tstep : int
            Time step for the old variable
        old : bool
            Whether to read var or old_var

        Returns
        -------
        np.array
            Data from csv file
        """

        var = 'old_var' if old else 'var'
        n = tstep if old else tstep - 1

        file = f'coupled_old_vector_nodal_out_{var}_{str(n).zfill(4)}.csv'

        return np.loadtxt(file,
                          delimiter=',',
                          skiprows=1)

    def test_old_vector(self):
        """Compares old value at present step with value at previous step"""
        for i in range(2,10):
            data_old = self.read_time_step(i, True)
            data_new = self.read_time_step(i, False)

            assert np.allclose(data_old, data_new)

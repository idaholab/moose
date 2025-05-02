"""Contains test cases for CoupledVectorValueOld function"""

import unittest

import vtk
import numpy as np

from vtk.util.numpy_support import vtk_to_numpy


class TestMonomialCoupledVectorValueOld(unittest.TestCase):
    """Test for constant monomials."""
    def setUp(self):
        """ Set up Exodus reader"""
        self._reader = vtk.vtkExodusIIReader()
        self._reader.SetFileName("coupled_old_vector_out.e")
        self._reader.UpdateInformation()

        self._reader.SetElementResultArrayStatus("var_", 1)
        self._reader.SetElementResultArrayStatus("old_var_", 1)


    def read_time_step(self, tstep: int) -> vtk.vtkMultiBlockDataSet:
        """Set reader time step and read data

        Parameters
        ----------
        tstep : int
            Time step

        Returns
        -------
        vtk.vtkMultiBlockDataSet
            Data at time step `tstep`
        """
        self._reader.SetTimeStep(tstep)
        self._reader.Update()
        return self._reader.GetOutputDataObject(0)

    def test_old_vector(self):
        """Compares old value at present step with value at previous step"""
        num_tstep = self._reader.GetNumberOfTimeSteps()
        for i in range(1,num_tstep):
            exo_data_old = self.read_time_step(i-1)

            ugrid: vtk.vtkUnstructuredGrid = exo_data_old.GetBlock(0).GetBlock(0)
            cell_data = ugrid.GetCellData()
            var = vtk_to_numpy(cell_data.GetAbstractArray("var_"))

            exo_data_new = self.read_time_step(i)
            ugrid: vtk.vtkUnstructuredGrid = exo_data_new.GetBlock(0).GetBlock(0)
            cell_data = ugrid.GetCellData()
            old_var = cell_data.GetAbstractArray("old_var_")

            assert np.allclose(var, old_var)



class TestNodalCoupledVectorValueOld(unittest.TestCase):
    """Test for nodal variables."""
    def setUp(self):
        """ Set up Exodus reader"""
        self._reader = vtk.vtkExodusIIReader()
        self._reader.SetFileName("coupled_old_vector_nodal_out.e")
        self._reader.UpdateInformation()

        self._reader.SetPointResultArrayStatus("var_", 1)
        self._reader.SetPointResultArrayStatus("old_var_", 1)

    def read_time_step(self, tstep: int) -> vtk.vtkMultiBlockDataSet:
        """Set reader time step and read data

        Parameters
        ----------
        tstep : int
            Time step

        Returns
        -------
        vtk.vtkMultiBlockDataSet
            Data at time step `tstep`
        """

        self._reader.SetTimeStep(tstep)
        self._reader.Update()
        return self._reader.GetOutputDataObject(0)

    def test_old_vector(self):
        """Compares old value at present step with value at previous step"""
        num_tstep = self._reader.GetNumberOfTimeSteps()
        for i in range(1,num_tstep):
            print(i)
            exo_data_old = self.read_time_step(i-1)

            ugrid: vtk.vtkUnstructuredGrid = exo_data_old.GetBlock(0).GetBlock(0)
            point_data = ugrid.GetPointData()
            var = vtk_to_numpy(point_data.GetAbstractArray("var_"))

            exo_data_new = self.read_time_step(i)
            ugrid: vtk.vtkUnstructuredGrid = exo_data_new.GetBlock(0).GetBlock(0)
            point_data = ugrid.GetPointData()
            old_var = point_data.GetAbstractArray("old_var_")

            assert np.allclose(var, old_var)

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputMatrixSampler.h"

registerMooseObjectAliased("StochasticToolsApp", InputMatrixSampler, "InputMatrix");

InputParameters
InputMatrixSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Sampler that utilizes a sampling matrix defined at input.");
  params.addRequiredParam<RealEigenMatrix>("matrix", "Sampling matrix.");
  return params;
}

InputMatrixSampler::InputMatrixSampler(const InputParameters & parameters)
  : Sampler(parameters), _data(getParam<RealEigenMatrix>("matrix"))
{
  setNumberOfRows(_data.rows());
  setNumberOfCols(_data.cols());
}

Real
InputMatrixSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // Checks to make sure that the row and column indices are not out of bounds
  // Static cast to avoid compiler warning and not lose information
  mooseAssert(static_cast<Real>(row_index) < static_cast<Real>(_data.rows()),
              "row_index cannot be out of bounds of the data.");
  mooseAssert(static_cast<Real>(col_index) < static_cast<Real>(_data.cols()),
              "col_index cannot be out of bounds of the data.");

  return _data(row_index, col_index);
}

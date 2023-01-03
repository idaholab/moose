//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"

/**
 * A class used to construct a sampling matrix from input
 */
class InputMatrixSampler : public Sampler
{
public:
  static InputParameters validParams();

  InputMatrixSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

private:
  /// Data read from input
  const RealEigenMatrix & _data;
};

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

class VectorPostprocessorSampler : public Sampler
{
public:
  static InputParameters validParams();

  VectorPostprocessorSampler(const InputParameters & parameters);

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Set the number of rows and columns after postprocessor data is filled
  void executeSetUp() override;

private:
  /// Storage of VPP data
  std::vector<const std::vector<Real> *> _data;
};

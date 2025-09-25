//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

/**
 *  ParisLawVpp is a VectorPostprocessor that compute fracture growth size and number of cycles
 */

class ParisLawVpp : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();
  ParisLawVpp(const InputParameters & parameters);
  virtual void execute() override;
  virtual void initialize() override {}

protected:
  /// Length of crack growth for each fracture integral value
  const Real _max_growth_size;
  /// Paris law parameters
  const Real _paris_law_c;
  const Real _paris_law_m;

  /// The name of the VectorPostprocessor with fracture integral values
  const VectorPostprocessorValue & _ki_vpp;
  const VectorPostprocessorValue & _kii_vpp;
  ///@{ The variables with the x, y, z, id data where ki was computed
  const VectorPostprocessorValue &  _ki_x;
  const VectorPostprocessorValue &  _ki_y;
  const VectorPostprocessorValue &  _ki_z;
  const VectorPostprocessorValue &  _ki_id;
  ///@}

  /// Number of cycles vector to reach max_growth_size postprocessor
  VectorPostprocessorValue &  _dn;

  /// growth rate vector postprocessor
  VectorPostprocessorValue & _growth_increment;

  ///@{ The variables with the x, y, z data for output
  VectorPostprocessorValue &  _x;
  VectorPostprocessorValue &  _y;
  VectorPostprocessorValue &  _z;
  VectorPostprocessorValue &  _id;
  ///@}
};

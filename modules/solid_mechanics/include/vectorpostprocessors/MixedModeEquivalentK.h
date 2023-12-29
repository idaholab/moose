//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"

// Forward Declarations

class MixedModeEquivalentK : public GeneralVectorPostprocessor
{
public:
  static InputParameters validParams();

  MixedModeEquivalentK(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();

protected:
  const VectorPostprocessorName _ki_vpp_name;
  const VectorPostprocessorName _kii_vpp_name;
  const VectorPostprocessorName _kiii_vpp_name;
  const std::string _ki_vector_name;
  const std::string _kii_vector_name;
  const std::string _kiii_vector_name;
  const VectorPostprocessorValue & _ki_value;
  const VectorPostprocessorValue & _kii_value;
  const VectorPostprocessorValue & _kiii_value;
  const VectorPostprocessorValue & _x_value;
  const VectorPostprocessorValue & _y_value;
  const VectorPostprocessorValue & _z_value;
  const VectorPostprocessorValue & _position_value;
  const Real & _poissons_ratio;
  unsigned int _ring_index;

  VectorPostprocessorValue & _x;
  VectorPostprocessorValue & _y;
  VectorPostprocessorValue & _z;
  VectorPostprocessorValue & _position;
  VectorPostprocessorValue & _k_eq;
};

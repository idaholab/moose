//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "GrainTracker.h"
#include "EulerAngleProvider.h"

class ComputePolycrystalGBAnisotropy : public Material
{
public:
  static InputParameters validParams();

  ComputePolycrystalGBAnisotropy(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// Grain tracker object
  const GrainTracker & _grain_tracker;

  /// object providing the Euler angles
  const EulerAngleProvider & _euler;
  
  /// Number of order parameters
  const unsigned int _op_num;

  /// Order parameters
  const std::vector<const VariableValue *> _vals;

  MaterialProperty<Real> & _delta_theta; // GB mobility
};
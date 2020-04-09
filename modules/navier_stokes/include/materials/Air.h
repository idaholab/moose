//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NavierStokesMaterial.h"

// Forward Declarations

// Class for Air with constant properties
// TODO: Add thermal conductivity
class Air : public NavierStokesMaterial
{
public:
  static InputParameters validParams();

  Air(const InputParameters & parameters);

protected:
  virtual void computeProperties();

  // The dynamic viscosity we will report back to the base class in computeProperties()
  // This can be set in the input file, otherwise a default value for air at 300K will be
  // used.  Some tabulated values for air at different temperatures are given below:
  //
  // 100K, 0.6924e-5
  // 150K, 1.0283e-5
  // 200K, 1.3289e-5
  // 250K, 1.4880e-5
  // 300K, 1.9830e-5
  // 350K, 2.0750e-5
  // 400K, 2.2860e-5
  // 450K, 2.4840e-5
  // 500K, 2.6710e-5
  // 550K, 2.8480e-5
  // 600K, 3.0180e-5
  Real _mu;
};

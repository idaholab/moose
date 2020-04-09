//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"
#include "RichardsVarNames.h"

// Forward Declarations

/**
 * This postprocessor computes the fluid mass by integrating the density over the volume
 *
 */
class RichardsMass : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  RichardsMass(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// userobject that holds Richards variable names
  const RichardsVarNames & _richards_name_UO;

  /// Richards variable number that we want the mass for
  unsigned int _pvar;

  /// Mass, or vector of masses in multicomponent situation
  const MaterialProperty<std::vector<Real>> & _mass;
};

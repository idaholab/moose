//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeKernel.h"
#include "Material.h"
#include "RichardsDensity.h"

// Forward Declarations

/**
 * fluid_mass/dt lumped to the nodes
 */
class Q2PNodalMass : public TimeKernel
{
public:
  static InputParameters validParams();

  Q2PNodalMass(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const RichardsDensity & _density;

  /// the other variable (this is porepressure if the Variable is saturation)
  const VariableValue & _other_var_nodal;

  /// variable number of the other variable
  unsigned int _other_var_num;

  /// whether the "other variable" is actually porepressure
  bool _var_is_pp;

  /// current value of the porosity
  const MaterialProperty<Real> & _porosity;
};

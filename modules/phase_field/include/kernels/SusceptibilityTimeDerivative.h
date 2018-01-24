//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SUSCEPTIBILITYTIMEDERIVATIVE_H
#define SUSCEPTIBILITYTIMEDERIVATIVE_H

#include "TimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"
// Forward Declaration
class SusceptibilityTimeDerivative;

template <>
InputParameters validParams<SusceptibilityTimeDerivative>();
/**
 * This calculates the time derivative for a variable multiplied by a generalized susceptibility
 **/
class SusceptibilityTimeDerivative
    : public DerivativeMaterialInterface<JvarMapKernelInterface<TimeDerivative>>
{
public:
  SusceptibilityTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// susceptibility
  const MaterialProperty<Real> & _Chi;

  /// susceptibility derivative w.r.t. the kernel variable
  const MaterialProperty<Real> & _dChidu;

  /// susceptibility derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<Real> *> _dChidarg;
};

#endif // SUSCEPTIBILITYTIMEDERIVATIVE_H

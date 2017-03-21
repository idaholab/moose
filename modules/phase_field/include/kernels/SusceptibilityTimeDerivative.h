/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

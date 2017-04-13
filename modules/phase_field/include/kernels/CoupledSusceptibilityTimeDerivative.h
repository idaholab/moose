/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDSUSCEPTIBILITYTIMEDERIVATIVE_H
#define COUPLEDSUSCEPTIBILITYTIMEDERIVATIVE_H

#include "CoupledTimeDerivative.h"
#include "JvarMapInterface.h"
#include "DerivativeMaterialInterface.h"
// Forward Declaration
class CoupledSusceptibilityTimeDerivative;

template <>
InputParameters validParams<CoupledSusceptibilityTimeDerivative>();

/**
 * This calculates a modified coupled time derivative that multiply the time derivative of a coupled
 *variable by a function of the variables
 **/
class CoupledSusceptibilityTimeDerivative
    : public DerivativeMaterialInterface<JvarMapKernelInterface<CoupledTimeDerivative>>
{
public:
  CoupledSusceptibilityTimeDerivative(const InputParameters & parameters);
  virtual void initialSetup();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// The function multiplied by the coupled time derivative
  const MaterialProperty<Real> & _F;

  /// function derivative w.r.t. the kernel variable
  const MaterialProperty<Real> & _dFdu;

  /// function derivatives w.r.t. coupled variables
  std::vector<const MaterialProperty<Real> *> _dFdarg;
};

#endif // COUPLEDSUSCEPTIBILITYTIMEDERIVATIVE_H

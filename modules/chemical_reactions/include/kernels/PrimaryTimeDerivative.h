/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PRIMARYTIMEDERIVATIVE
#define PRIMARYTIMEDERIVATIVE

#include "TimeDerivative.h"

// Forward Declaration
class PrimaryTimeDerivative;

template<>
InputParameters validParams<PrimaryTimeDerivative>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 * storage * delta pressure / delta t
 */
class PrimaryTimeDerivative : public TimeDerivative
{
public:

  PrimaryTimeDerivative(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Material property of porosity
  const MaterialProperty<Real> & _porosity;
};

#endif // PRIMARYTIMEDERIVATIVE

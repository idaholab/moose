/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSSEFFPRIMEAUX_H
#define RICHARDSSEFFPRIMEAUX_H

#include "AuxKernel.h"

#include "RichardsSeff.h"

// Forward Declarations
class RichardsSeffPrimeAux;

template <>
InputParameters validParams<RichardsSeffPrimeAux>();

/**
 * Calculates derivative of effective saturation wrt a specified porepressure
 */
class RichardsSeffPrimeAux : public AuxKernel
{
public:
  RichardsSeffPrimeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /**
   * The user object that defines effective saturation
   * as function of porepressure (or porepressures in the
   * multiphase situation)
   */
  const RichardsSeff & _seff_UO;

  /**
   * AuxKernel calculates d^2(Seff)/dP_wrt1
   * so wrt1 is the porepressure number to differentiate wrt to
   */
  int _wrt1;

  /**
   * the porepressure values (this will be length N
   * where N is the number of arguments that the _seff_UO requires)
   * Eg, for twophase simulations N=2
   */
  std::vector<const VariableValue *> _pressure_vals;

  /// array of derivtives: This auxkernel returns _mat[_wrt1]
  std::vector<Real> _mat;
};

#endif // RICHARDSSEFFPRIMEAUX_H

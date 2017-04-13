/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef VARIABLETIMEINTEGRATIONAUX_H
#define VARIABLETIMEINTEGRATIONAUX_H

#include "AuxKernel.h"

// forward declarations
class VariableTimeIntegrationAux;

template <>
InputParameters validParams<VariableTimeIntegrationAux>();

/**
 * An AuxKernel that can be used to integrate a field variable in time
 * using a variety of different integration methods.  The result is
 * stored in another field variable.
 */
class VariableTimeIntegrationAux : public AuxKernel
{
public:
  VariableTimeIntegrationAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  Real getIntegralValue();

  std::vector<const VariableValue *> _coupled_vars;
  Real _coef;
  unsigned int _order;
  std::vector<Real> _integration_coef;
};

#endif // VARIABLETIMEINTEGRATIONAUX_H

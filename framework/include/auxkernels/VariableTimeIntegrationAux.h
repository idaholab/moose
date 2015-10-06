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

//forward declarations
class VariableTimeIntegrationAux;

template<>
InputParameters validParams<VariableTimeIntegrationAux>();

class VariableTimeIntegrationAux :  public AuxKernel
{
public:
  VariableTimeIntegrationAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  Real getIntegralValue();

  std::vector<VariableValue *>  _coupled_vars;
  Real _coef;
  unsigned int _order;
  std::vector<Real> _integration_coef;

};

#endif // VARIABLETIMEINTEGRATIONAUX_H


/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COUPLEDCONVECTIVEFLUX_H
#define COUPLEDCONVECTIVEFLUX_H

#include "IntegratedBC.h"

class CoupledConvectiveFlux : public IntegratedBC
{
public:

  CoupledConvectiveFlux(const std::string & name, InputParameters parameters);
  virtual ~CoupledConvectiveFlux() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  VariableValue & _T_infinity;
  const Real _coefficient;
};

template<>
InputParameters validParams<CoupledConvectiveFlux>();

#endif //COUPLEDCONVECTIVEFLUX_H

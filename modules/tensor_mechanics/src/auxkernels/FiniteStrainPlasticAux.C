/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FiniteStrainPlasticAux.h"

template<>
InputParameters validParams<FiniteStrainPlasticAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

FiniteStrainPlasticAux::FiniteStrainPlasticAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _eqv_plastic_strain(getMaterialProperty<Real>("eqv_plastic_strain"))
{
}

Real
FiniteStrainPlasticAux::computeValue()
{
  return _eqv_plastic_strain[_qp];
}

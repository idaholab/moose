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
#include "VarCouplingMaterial.h"

template<>
InputParameters validParams<VarCouplingMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addParam<Real>("base", 0.0, "The baseline of the property");
  params.addParam<Real>("coef", 1.0, "The linear coefficient of the coupled var");
  return params;
}


VarCouplingMaterial::VarCouplingMaterial(const std::string & name, InputParameters parameters) :
    Material(name, parameters),
    _var(coupledValue("var")),
    _base(getParam<Real>("base")),
    _coef(getParam<Real>("coef")),
    _diffusion(declareProperty<Real>("diffusion"))
{
}

void
VarCouplingMaterial::computeQpProperties()
{
  _diffusion[_qp] = _base + _coef * _var[_qp];
}

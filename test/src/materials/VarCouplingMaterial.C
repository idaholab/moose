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

template <>
InputParameters
validParams<VarCouplingMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addParam<Real>("base", 0.0, "The baseline of the property");
  params.addParam<Real>("coef", 1.0, "The linear coefficient of the coupled var");
  params.addParam<bool>(
      "declare_old", false, "When True the old value for the material property is declared.");
  return params;
}

VarCouplingMaterial::VarCouplingMaterial(const InputParameters & parameters)
  : Material(parameters),
    _var(coupledValue("var")),
    _base(getParam<Real>("base")),
    _coef(getParam<Real>("coef")),
    _diffusion(declareProperty<Real>("diffusion")),
    _diffusion_old(getParam<bool>("declare_old") ? &getMaterialPropertyOld<Real>("diffusion")
                                                 : nullptr)
{
}

void
VarCouplingMaterial::initQpStatefulProperties()
{
  _diffusion[_qp] = _var[_qp];
}

void
VarCouplingMaterial::computeQpProperties()
{
  // If "declare_old" is set, then just use it. The test associated is checking that
  // initQpStatefulProperties can use a coupledValue
  if (_diffusion_old)
    _diffusion[_qp] = (*_diffusion_old)[_qp];
  else
    _diffusion[_qp] = _base + _coef * _var[_qp];
}

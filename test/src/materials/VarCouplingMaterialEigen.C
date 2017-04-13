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
#include "VarCouplingMaterialEigen.h"

template <>
InputParameters
validParams<VarCouplingMaterialEigen>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("var", "The variable to be coupled in");
  params.addRequiredParam<std::string>("material_prop_name", "Property name");
  return params;
}

VarCouplingMaterialEigen::VarCouplingMaterialEigen(const InputParameters & parameters)
  : Material(parameters),
    _var(coupledValue("var")),
    _var_old(coupledValueOld("var")),
    _propname(getParam<std::string>("material_prop_name")),
    _mat(declareProperty<Real>(_propname)),
    _mat_old(declareProperty<Real>(_propname + "_old"))
{
}

void
VarCouplingMaterialEigen::computeQpProperties()
{
  _mat[_qp] = _var[_qp];
  _mat_old[_qp] = _var_old[_qp];
}

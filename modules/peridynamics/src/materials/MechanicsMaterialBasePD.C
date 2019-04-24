//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsMaterialBasePD.h"
#include "MooseVariableFEBase.h"

template <>
InputParameters
validParams<MechanicsMaterialBasePD>()
{
  InputParameters params = validParams<MaterialBasePD>();
  params.addClassDescription("Base class for Peridynamic mechanic materials");

  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable name for the displacements");
  params.addParam<VariableName>("temperature", "Nonlinear variable name for the temperature");

  return params;
}

MechanicsMaterialBasePD::MechanicsMaterialBasePD(const InputParameters & parameters)
  : MaterialBasePD(parameters),
    _has_temp(isParamValid("temperature")),
    _temp_var(_has_temp ? &_subproblem.getVariable(_tid, getParam<VariableName>("temperature"))
                        : NULL),
    _bond_status_var(_subproblem.getVariable(_tid, "bond_status")),
    _total_stretch(declareProperty<Real>("total_stretch")),
    _mechanical_stretch(declareProperty<Real>("mechanical_stretch"))
{
  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));
  if (_dim != nl_vnames.size())
    mooseError("Size of displacements vector is different from the mesh dimension!");

  for (unsigned int i = 0; i < nl_vnames.size(); ++i)
    _disp_var.push_back(&_subproblem.getVariable(_tid, nl_vnames[i]));
}

void
MechanicsMaterialBasePD::computeProperties()
{
  MaterialBasePD::computeProperties();

  // current length of a EDGE2 element
  computeBondCurrentLength();
  computeBondStretch();
}

void
MechanicsMaterialBasePD::computeBondCurrentLength()
{
  RealGradient dxyz;
  dxyz.zero();

  for (unsigned int i = 0; i < _dim; ++i)
  {
    dxyz(i) =
        (*_current_elem->node_ptr(1))(i) + _disp_var[i]->getNodalValue(*_current_elem->node_ptr(1));
    dxyz(i) -=
        (*_current_elem->node_ptr(0))(i) + _disp_var[i]->getNodalValue(*_current_elem->node_ptr(0));
  }

  _current_length = dxyz.norm();
}

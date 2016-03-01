//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicsMaterialBasePD.h"
#include "MooseVariable.h"

InputParameters
MechanicsMaterialBasePD::validParams()
{
  InputParameters params = PeridynamicsMaterialBase::validParams();
  params.addClassDescription("Base class for Peridynamic mechanic materials");

  params.addRequiredCoupledVar("displacements", "Nonlinear variable name for the displacements");
  params.addCoupledVar("temperature", "Nonlinear variable name for the temperature");

  return params;
}

MechanicsMaterialBasePD::MechanicsMaterialBasePD(const InputParameters & parameters)
  : PeridynamicsMaterialBase(parameters),
    _has_temp(isCoupled("temperature")),
    _temp_var(_has_temp ? getVar("temperature", 0) : nullptr),
    _bond_status_var(&_subproblem.getStandardVariable(_tid, "bond_status")),
    _total_stretch(declareProperty<Real>("total_stretch")),
    _mechanical_stretch(declareProperty<Real>("mechanical_stretch"))
{
  if (_dim != coupledComponents("displacements"))
    mooseError("Size of displacements vector is different from the mesh dimension!");

  for (unsigned int i = 0; i < coupledComponents("displacements"); ++i)
    _disp_var.push_back(getVar("displacements", i));
}

void
MechanicsMaterialBasePD::computeBondCurrentLength()
{
  RealGradient dxyz;
  dxyz.zero();

  for (unsigned int i = 0; i < _dim; ++i)
  {
    dxyz(i) = (_pdmesh.getNodeCoord(_current_elem->node_id(1)))(i) +
              _disp_var[i]->getNodalValue(*_current_elem->node_ptr(1));
    dxyz(i) -= (_pdmesh.getNodeCoord(_current_elem->node_id(0)))(i) +
               _disp_var[i]->getNodalValue(*_current_elem->node_ptr(0));
  }

  _current_len = dxyz.norm();
}

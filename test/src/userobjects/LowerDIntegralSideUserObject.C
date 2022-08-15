//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LowerDIntegralSideUserObject.h"

registerMooseObject("MooseTestApp", LowerDIntegralSideUserObject);

InputParameters
LowerDIntegralSideUserObject::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addCoupledVar("lowerd_variable", "lower d variable name");
  return params;
}

LowerDIntegralSideUserObject::LowerDIntegralSideUserObject(const InputParameters & params)
  : SideUserObject(params), _lower_d_value(&getVar("lowerd_variable", 0)->slnLower())
{
}

void
LowerDIntegralSideUserObject::initialSetup()
{
}

void
LowerDIntegralSideUserObject::initialize()
{
  _integral_value = 0;
}

void
LowerDIntegralSideUserObject::execute()
{
  const auto neighbor = _current_elem->neighbor_ptr(_current_side);
  const bool upwind_elem = !neighbor || _current_elem->id() < neighbor->id();
  const Elem * current_lower_d_elem =
      upwind_elem ? _fe_problem.mesh().getLowerDElem(_current_elem, _current_side)
                  : _fe_problem.mesh().getLowerDElem(neighbor,
                                                     neighbor->which_neighbor_am_i(_current_elem));
  _fe_problem.reinitLowerDElem(current_lower_d_elem, _tid);
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    const auto t = _JxW[qp] * _coord[qp];
    _integral_value += (*_lower_d_value)[qp] * t;
  }
}

void
LowerDIntegralSideUserObject::threadJoin(const UserObject & y)
{
  const LowerDIntegralSideUserObject & x = dynamic_cast<const LowerDIntegralSideUserObject &>(y);
  _integral_value += x._integral_value;
}

void
LowerDIntegralSideUserObject::finalize()
{
}

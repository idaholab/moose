//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPressureAux.h"

#include "NodalArea.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("ContactApp", ContactPressureAux);

InputParameters
ContactPressureAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "paired_boundary",
      "The set of boundaries in contact with those specified in 'boundary'. Ordering must be "
      "consistent with that in 'boundary'.");
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR;
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  params.addParam<MooseEnum>("order", orders, "The finite element order: " + orders.getRawNames());

  params.addClassDescription("Computes the contact pressure from the contact force and nodal area");

  return params;
}

ContactPressureAux::ContactPressureAux(const InputParameters & params)
  : AuxKernel(params),
    _nodal_area(coupledValue("nodal_area")),
    _number_pairs(getParam<std::vector<BoundaryName>>("paired_boundary").size()),
    _penetration_locators(_number_pairs)
{
  if (_number_pairs != getParam<std::vector<BoundaryName>>("boundary").size())
    paramError("boundary",
               "Boundary and paired boundary vectors are not the same size in the contact pressure "
               "auxiliary kernel. Please check your input");

  for (const auto i : make_range(_number_pairs))
    _penetration_locators[i] =
        &getPenetrationLocator(getParam<std::vector<BoundaryName>>("paired_boundary")[i],
                               getParam<std::vector<BoundaryName>>("boundary")[i],
                               Utility::string_to_enum<Order>(getParam<MooseEnum>("order")));
}

ContactPressureAux::~ContactPressureAux() {}

Real
ContactPressureAux::computeValue()
{
  Real value(0);
  const Real area = _nodal_area[_qp];
  const PenetrationInfo * penetration_info(nullptr);

  for (const auto i : make_range(_number_pairs))
  {
    const auto it = _penetration_locators[i]->_penetration_info.find(_current_node->id());
    if (it != _penetration_locators[i]->_penetration_info.end())
      penetration_info = it->second;

    if (penetration_info && area != 0)
      value -= (penetration_info->_contact_force * penetration_info->_normal) / area;

    penetration_info = nullptr;
  }

  return value;
}

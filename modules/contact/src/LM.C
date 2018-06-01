//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LM.h"
#include "PenetrationInfo.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

registerMooseObject("ContactApp", LM);

template <>
InputParameters
validParams<LM>()
{
  InputParameters params = validParams<NodalKernel>();
  params.addRequiredParam<BoundaryName>("slave", "The boundary ID associated with the slave side");
  params.addRequiredParam<BoundaryName>("master",
                                        "The boundary ID associated with the master side");
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");
  params.addParam<MooseEnum>("order", orders, "The finite element order used for projections");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

LM::LM(const InputParameters & parameters)
  : NodalKernel(parameters),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("master"),
                              getParam<BoundaryName>("slave"),
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order"))))
{
}

Real
LM::computeQpResidual()
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = -pinfo->_distance;
      Real b = _u[_qp];
      return a + b - std::sqrt(a * a + b * b);
    }
  }
  return 0;
}

Real
LM::computeQpJacobian()
{
  std::map<dof_id_type, PenetrationInfo *>::iterator found =
      _penetration_locator._penetration_info.find(_current_node->id());
  if (found != _penetration_locator._penetration_info.end())
  {
    PenetrationInfo * pinfo = found->second;
    if (pinfo != NULL)
    {
      Real a = pinfo->_distance;
      Real b = _u[_qp];
      return 1. + b / std::sqrt(a * a + b * b);
    }
  }
  return 0;
}

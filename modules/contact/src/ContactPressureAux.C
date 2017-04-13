/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ContactPressureAux.h"

#include "NodalArea.h"
#include "PenetrationLocator.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<ContactPressureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("nodal_area", "The nodal area");
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary to be penetrated");
  MooseUtils::setExecuteOnFlags(params, 1, EXEC_NONLINEAR);
  return params;
}

ContactPressureAux::ContactPressureAux(const InputParameters & params)
  : AuxKernel(params),
    _nodal_area(coupledValue("nodal_area")),
    _penetration_locator(
        getPenetrationLocator(getParam<BoundaryName>("paired_boundary"),
                              getParam<std::vector<BoundaryName>>("boundary")[0],
                              Utility::string_to_enum<Order>(getParam<MooseEnum>("order"))))
{
}

ContactPressureAux::~ContactPressureAux() {}

Real
ContactPressureAux::computeValue()
{
  Real value(0);
  const Real area = _nodal_area[_qp];
  const PenetrationInfo * pinfo(NULL);

  const auto it = _penetration_locator._penetration_info.find(_current_node->id());
  if (it != _penetration_locator._penetration_info.end())
    pinfo = it->second;

  if (pinfo && area != 0)
    value = -(pinfo->_contact_force * pinfo->_normal) / area;

  return value;
}

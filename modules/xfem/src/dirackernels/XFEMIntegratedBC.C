//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMIntegratedBC.h"
#include "ElementPairLocator.h"

InputParameters
XFEMIntegratedBC::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addClassDescription("Applies a pressure on an interface cut by XFEM.");
  params.addRequiredParam<UserObjectName>("geometric_cut_userobject",
                                          "Name of GeometricCutUserObject associated with this BC");
  return params;
}

XFEMIntegratedBC::XFEMIntegratedBC(const InputParameters & parameters)
  : DiracKernel(parameters),
    _gcuo(getUserObject<GeometricCutUserObject>("geometric_cut_userobject")),
    _interface_id(_gcuo.getInterfaceID()),
    _element_pair_locators(_subproblem.geomSearchData()._element_pair_locators)
{
}

void
XFEMIntegratedBC::addPoints()
{
  _elem_qp_normal.clear();
  _elem_qp_JxW.clear();

  // Get element pairs on the interface
  const ElementPairLocator & elem_pair_loc = *_element_pair_locators.at(_interface_id);

  // Go over element pairs
  auto elem_pairs = elem_pair_loc.getElemPairs();
  for (auto elem_pair : elem_pairs)
  {
    const Elem * elem1 = elem_pair.first;
    const Elem * elem2 = elem_pair.second;
    const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(elem_pair);

    for (unsigned int i = 0; i < info._elem1_constraint_q_point.size(); ++i)
    {
      _elem_qp_normal[elem1][i] = info._elem1_normal;
      _elem_qp_normal[elem2][i] = info._elem2_normal;
      _elem_qp_JxW[elem1][i] = info._elem1_constraint_JxW[i];
      _elem_qp_JxW[elem2][i] = info._elem2_constraint_JxW[i];
      addPoint(elem1, info._elem1_constraint_q_point[i]);
      addPoint(elem2, info._elem2_constraint_q_point[i]);
    }
  }
}

void
XFEMIntegratedBC::reinitQp()
{
  _current_point = _physical_point[_qp];
  _interface_normal = _elem_qp_normal[_current_elem][_qp];
  _integration_factor = _elem_qp_JxW[_current_elem][_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMPressure.h"
#include "Function.h"
#include "GeometricSearchData.h"
#include "ElementPairLocator.h"
#include "FEProblem.h"

registerMooseObject("XFEMApp", XFEMPressure);

InputParameters
XFEMPressure::validParams()
{
  InputParameters params = DiracKernel::validParams();
  params.addRequiredParam<unsigned int>("component", "The component for the pressure");
  params.addParam<Real>("factor", 1.0, "The magnitude to use in computing the pressure");
  params.addParam<FunctionName>("function", "The function that describes the pressure");
  params.addClassDescription("Applies a pressure on an interface cut by XFEM.");
  return params;
}

XFEMPressure::XFEMPressure(const InputParameters & parameters)
  : DiracKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _factor(getParam<Real>("factor")),
    _function(isParamValid("function") ? &getFunction("function") : NULL)
{
  GeometricSearchData & geo_search_data = _subproblem.geomSearchData();
  _element_pair_locators = &geo_search_data._element_pair_locators;
}

void
XFEMPressure::addPoints()
{
  _elem_qp_normal.clear();
  _elem_qp_JxW.clear();

  for (std::map<unsigned int, std::shared_ptr<ElementPairLocator>>::iterator it_epl =
           _element_pair_locators->begin();
       it_epl != _element_pair_locators->end();
       ++it_epl)
  {
    ElementPairLocator & elem_pair_loc = *it_epl->second;
    // go over pair elements
    const std::list<std::pair<const Elem *, const Elem *>> & elem_pairs =
        elem_pair_loc.getElemPairs();
    for (std::list<std::pair<const Elem *, const Elem *>>::const_iterator it_ep =
             elem_pairs.begin();
         it_ep != elem_pairs.end();
         ++it_ep)
    {
      const Elem * elem1 = it_ep->first;
      const Elem * elem2 = it_ep->second;
      const ElementPairInfo & info = elem_pair_loc.getElemPairInfo(*it_ep);

      for (unsigned int i = 0; i < info._elem1_constraint_q_point.size(); ++i)
      {
        _elem_qp_normal[elem1][i] = -info._elem1_normal;
        _elem_qp_normal[elem2][i] = -info._elem2_normal;
        _elem_qp_JxW[elem1][i] = info._elem1_constraint_JxW[i];
        _elem_qp_JxW[elem2][i] = info._elem2_constraint_JxW[i];
        addPoint(elem1, info._elem1_constraint_q_point[i]);
        addPoint(elem2, info._elem2_constraint_q_point[i]);
      }
    }
  }
}

Real
XFEMPressure::computeQpResidual()
{
  Real factor = _factor;

  if (_function)
    factor *= _function->value(_t, _current_point);

  Point normal = _elem_qp_normal[_current_elem][_qp];
  Real JxW = _elem_qp_JxW[_current_elem][_qp];

  return -factor * JxW * (normal(_component) * _test[_i][_qp]);
}

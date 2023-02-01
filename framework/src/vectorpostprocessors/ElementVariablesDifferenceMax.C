//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementVariablesDifferenceMax.h"

// MOOSE includes
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", ElementVariablesDifferenceMax);

InputParameters
ElementVariablesDifferenceMax::validParams()
{
  InputParameters params = ElementVectorPostprocessor::validParams();
  params.addClassDescription("Computes the largest difference between two variable fields.");
  params.addRequiredCoupledVar(
      "compare_a",
      "The first variable to evaluate the difference with, performed as \"compare_a - compare_b\"");
  params.addRequiredCoupledVar("compare_b",
                               "The second variable to evaluate the difference with, "
                               "performed as \"compare_a - compare_b\"");

  params.addParam<bool>(
      "furthest_from_zero", false, "Find the difference with the highest absolute value");

  // The value from this VPP is naturally already on every processor
  // TODO: Make this not the case!  See #11415
  params.set<bool>("_auto_broadcast") = false;

  return params;
}

enum CollectionOfAllValuesIntoAVector
{
  MAXIMUM_DIFFERENCE,
  MAXIMUM_DIFFERENCE_A_VALUE,
  MAXIMUM_DIFFERENCE_B_VALUE,
  MAXIMUM_DIFFERENCE_X,
  MAXIMUM_DIFFERENCE_Y,
  MAXIMUM_DIFFERENCE_Z,
  SIZE
};

ElementVariablesDifferenceMax::ElementVariablesDifferenceMax(const InputParameters & parameters)
  : ElementVectorPostprocessor(parameters),
    _a(coupledValue("compare_a")),
    _b(coupledValue("compare_b")),
    _a_value(declareVector(coupledName("compare_a"))),
    _b_value(declareVector(coupledName("compare_b"))),
    _max_difference(declareVector("Difference")),
    _position_x(declareVector("X")),
    _position_y(declareVector("Y")),
    _position_z(declareVector("Z")),
    _furthest_from_zero(getParam<bool>("furthest_from_zero"))
{
  // These are all single-value arrays
  _a_value.resize(1);
  _b_value.resize(1);
  _max_difference.resize(1);
  _position_x.resize(1);
  _position_y.resize(1);
  _position_z.resize(1);

  _all.resize(CollectionOfAllValuesIntoAVector::SIZE);
}

void
ElementVariablesDifferenceMax::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    // Get the difference
    const Real difference = _furthest_from_zero ? std::abs(_a[qp] - _b[qp]) : _a[qp] - _b[qp];

    // Assign the appropriate values if a new maximum is found
    if (difference > _all[MAXIMUM_DIFFERENCE])
    {
      _all[MAXIMUM_DIFFERENCE] = difference;
      _all[MAXIMUM_DIFFERENCE_A_VALUE] = _a[qp];
      _all[MAXIMUM_DIFFERENCE_B_VALUE] = _b[qp];

      _all[MAXIMUM_DIFFERENCE_X] = _q_point[qp](0);
      _all[MAXIMUM_DIFFERENCE_Y] = _q_point[qp](1);
      _all[MAXIMUM_DIFFERENCE_Z] = _q_point[qp](2);
    }
  }
}

void
ElementVariablesDifferenceMax::finalize()
{
  // Gather all the parameters based on the maximum difference
  gatherProxyValueMax(_all[MAXIMUM_DIFFERENCE], _all);

  _max_difference[0] = _all[MAXIMUM_DIFFERENCE];
  _a_value[0] = _all[MAXIMUM_DIFFERENCE_A_VALUE];
  _b_value[0] = _all[MAXIMUM_DIFFERENCE_B_VALUE];
  _position_x[0] = _all[MAXIMUM_DIFFERENCE_X];
  _position_y[0] = _all[MAXIMUM_DIFFERENCE_Y];
  _position_z[0] = _all[MAXIMUM_DIFFERENCE_Z];
}

void
ElementVariablesDifferenceMax::initialize()
{
  _all[MAXIMUM_DIFFERENCE] = 0.0;
  _all[MAXIMUM_DIFFERENCE_A_VALUE] = 0.0;
  _all[MAXIMUM_DIFFERENCE_B_VALUE] = 0.0;
  _all[MAXIMUM_DIFFERENCE_X] = 0.0;
  _all[MAXIMUM_DIFFERENCE_Y] = 0.0;
  _all[MAXIMUM_DIFFERENCE_Z] = 0.0;
}

void
ElementVariablesDifferenceMax::threadJoin(const UserObject & s)
{
  const ElementVariablesDifferenceMax & sibling =
      static_cast<const ElementVariablesDifferenceMax &>(s);

  if (_all[MAXIMUM_DIFFERENCE] < sibling._all[MAXIMUM_DIFFERENCE])
    _all = sibling._all;
}

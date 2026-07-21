//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterceptedElementModifier.h"

registerMooseObject("ShiftedBoundaryMethodApp", InterceptedElementModifier);

InputParameters
InterceptedElementModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();

  params.addClassDescription("Marks elements as inside, outside, or intercepted based on a given "
                             "distance function or geometry.");

  params.addParam<FunctionName>("signed_dist_function", "Signed Distance Function to evaluate");

  params.addRequiredParam<SubdomainID>("subdomain_id_inside", "ID for inside elements.");
  params.addRequiredParam<SubdomainID>("subdomain_id_outside", "ID for outside elements.");

  params.addParam<Real>("threshold", 0, "Threshold for inside/outside classification.");
  params.addRequiredParam<Real>("lambda", "Lambda for false intersection classification.");
  params.addRequiredParam<bool>("outer_boundary", "Flag for outer boundary handling.");
  params.addParam<bool>(
      "mark_neighbor_of_intercepted",
      false,
      "Whether we need to mark the element is the element near by the intercepted element.");

  params.addParam<int>("qrule_order",
                       6 /*16 gauss points for quad element*/,
                       "Quadrature order for generating the Gauss points to do the IN-OUT test to "
                       "test whether the intercepted element belong to false intercepted element.");

  params.addParam<UserObjectName>("in_out_test", "The name of the in-out test user object");

  // whether mark intercepted elements as well
  params.addParam<bool>(
      "mark_intercepted", false, "Whether we need to mark the intercepted elements.");
  params.addParam<SubdomainID>("subdomain_id_intercepted", -1, "ID for intercepted elements.");

  return params;
}

InterceptedElementModifier::InterceptedElementModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _parsed_function(isParamSetByUser("signed_dist_function")
                         ? &getFunctionByName(parameters.get<FunctionName>("signed_dist_function"))
                         : nullptr),
    _subdomain_id_inside(getParam<SubdomainID>("subdomain_id_inside")),
    _subdomain_id_outside(getParam<SubdomainID>("subdomain_id_outside")),
    _mark_intercepted(getParam<bool>("mark_intercepted")),
    _subdomain_id_intercepted(getParam<SubdomainID>("subdomain_id_intercepted")),
    _threshold(getParam<Real>("threshold")),
    _lambda(getParam<Real>("lambda")),
    _outer_boundary(getParam<bool>("outer_boundary")),
    _qrule_order(getParam<int>("qrule_order")),
    _in_out_test_base(isParamSetByUser("in_out_test")
                          ? &getUserObject<PointInPolyhedronCheckUO>("in_out_test")
                          : nullptr)
{
}

/// @brief Initial setup for the InterceptedElementModifier class to read in the Gmsh file
/// NOTE: this function should be overrided
void
InterceptedElementModifier::initialSetup()
{
  if (_in_out_test_base)
    _in_out_test_type = DistanceType::GEOMETRY;
  else if (_parsed_function)
    _in_out_test_type = DistanceType::SIGN_DISTANCE;
  else
    mooseError("InterceptedElementModifier: _in_out_test_base and _parsed_function are null!");
}

SubdomainID
InterceptedElementModifier::computeSubdomainID()
{
  const Elem * elem = this->_current_elem;
  if (!elem)
    mooseError("InterceptedElementModifier: _current_elem is null!");

  auto check_lambda_flags = [&](Real ratio_active) -> SubdomainID
  {
    if (_lambda == 0)
      return _subdomain_id_outside;
    if (_lambda == 1)
      return _subdomain_id_inside;
    return (1 - ratio_active > _lambda) ? _subdomain_id_outside : _subdomain_id_inside;
  };

  auto initFEBase = [&](const Elem * elem)
  {
    Order order = intToOrder(_qrule_order);
    FEType fe_type(elem->default_order(), LAGRANGE);
    std::unique_ptr<FEBase> fe(FEBase::build(elem->dim(), fe_type));
    QGauss qrule(elem->dim(), order);
    fe->get_xyz(); // this is very important, otherwise the quadrature points are not
                   // initialized
    fe->get_JxW();
    fe->attach_quadrature_rule(&qrule);
    fe->reinit(elem);
    return fe;
  };

  auto computeActiveAreaRatio =
      [&](const Elem * elem, const std::function<bool(const Point &)> & is_active)
  {
    auto fe = initFEBase(elem);
    const auto & JxW = fe->get_JxW();
    const auto & q_points = fe->get_xyz();
    double active_area = 0, total_area = 0;

    for (unsigned int i = 0; i < q_points.size(); ++i)
    {
      if (is_active(q_points[i]))
        active_area += JxW[i];
      total_area += JxW[i];
    }
    return active_area / total_area;
  };

  if (_in_out_test_type == DistanceType::SIGN_DISTANCE)
  {
    Real min_val = std::numeric_limits<Real>::max();
    Real max_val = std::numeric_limits<Real>::lowest();

    for (unsigned int node = 0; node < elem->n_nodes(); ++node)
    {
      Real val = _parsed_function->value(_t, elem->point(node));
      min_val = std::min(min_val, val);
      max_val = std::max(max_val, val);
    }

    if (max_val < _threshold)
      return _outer_boundary ? _subdomain_id_inside : _subdomain_id_outside;
    else if (min_val > _threshold)
      return _outer_boundary ? _subdomain_id_outside : _subdomain_id_inside;

    if (_mark_intercepted /*optional*/)
      return _subdomain_id_intercepted;

    auto is_active = [&](const Point & p)
    {
      Real val = _parsed_function->value(_t, p);
      return (_outer_boundary && val < _threshold) || (!_outer_boundary && val > _threshold);
    };

    Real ratio_active = computeActiveAreaRatio(elem, is_active);
    return check_lambda_flags(ratio_active);
  }
  else if (_in_out_test_type == DistanceType::GEOMETRY)
  {
    unsigned int active_nodes = 0;
    for (unsigned int node = 0; node < elem->n_nodes(); ++node)
      if (_in_out_test_base->ifInside(elem->point(node)))
        ++active_nodes;

    if (active_nodes == elem->n_nodes())
      return _outer_boundary ? _subdomain_id_inside : _subdomain_id_outside;
    else if (active_nodes == 0)
      return _outer_boundary ? _subdomain_id_outside : _subdomain_id_inside;

    if (_mark_intercepted /*optional*/)
      return _subdomain_id_intercepted;

    auto is_active = [&](const Point & p)
    {
      return (_outer_boundary && _in_out_test_base->ifInside(p)) ||
             (!_outer_boundary && !_in_out_test_base->ifInside(p));
    };

    Real ratio_active = computeActiveAreaRatio(elem, is_active);

    return check_lambda_flags(ratio_active);
  }
  else
  {
    mooseError("InterceptedElementModifier: Unknown PointInPolyhedronCheck type!");
  }

  return -1; // fallback (shouldn't reach)
}

Order
InterceptedElementModifier::intToOrder(int value)
{
  switch (value)
  {
    case 0:
      return CONSTANT;
    case 1:
      return FIRST;
    case 2:
      return SECOND;
    case 3:
      return THIRD;
    case 4:
      return FOURTH;
    case 5:
      return FIFTH;
    case 6:
      return SIXTH;
    case 7:
      return SEVENTH;
    case 8:
      return EIGHTH;
    case 9:
      return NINTH;
    case 10:
      return TENTH;
    default:
      throw std::invalid_argument("Unsupported Order value: " + std::to_string(value));
  }
}

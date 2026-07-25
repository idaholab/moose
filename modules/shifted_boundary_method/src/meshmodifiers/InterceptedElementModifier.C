//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterceptedElementModifier.h"
#include "SBMUtils.h"

registerMooseObject("ShiftedBoundaryMethodApp", InterceptedElementModifier);

InputParameters
InterceptedElementModifier::validParams()
{
  InputParameters params = SBMElementSubdomainModifierBase::validParams();

  params.addClassDescription("Marks elements as inside, outside, or intercepted based on a given "
                             "distance function or geometry.");

  params.addParam<FunctionName>("signed_dist_function", "Signed Distance Function to evaluate");

  params.addRequiredParam<SubdomainID>("subdomain_id_inside", "ID for inside elements.");
  params.addRequiredParam<SubdomainID>("subdomain_id_outside", "ID for outside elements.");

  params.addParam<Real>("threshold", 0, "Threshold for inside/outside classification.");
  params.addRequiredParam<bool>(
      "is_domain_inside_surface",
      "When true, the retained (inside) domain is the region enclosed by the surface (signed "
      "distance below 'threshold', or points reported inside by the in-out test); when false, the "
      "retained domain is the region outside the surface.");

  params.addParam<UserObjectName>("in_out_test", "The name of the in-out test user object");

  return params;
}

InterceptedElementModifier::InterceptedElementModifier(const InputParameters & parameters)
  : SBMElementSubdomainModifierBase(parameters),
    _parsed_function(isParamSetByUser("signed_dist_function")
                         ? &getFunctionByName(parameters.get<FunctionName>("signed_dist_function"))
                         : nullptr),
    _subdomain_id_inside(getParam<SubdomainID>("subdomain_id_inside")),
    _subdomain_id_outside(getParam<SubdomainID>("subdomain_id_outside")),
    _threshold(getParam<Real>("threshold")),
    _is_domain_inside_surface(getParam<bool>("is_domain_inside_surface")),
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
  // Run the base class setup (reinitialize subdomains/variables, moving boundary maps,
  // reinitialization strategy, and parameter consistency checks) before our own.
  SBMElementSubdomainModifierBase::initialSetup();

  // Exactly one geometry source must be provided.
  if (_in_out_test_base && _parsed_function)
    mooseError("InterceptedElementModifier: provide exactly one geometry source, but both "
               "'signed_dist_function' and 'in_out_test' were set.");

  if (_in_out_test_base)
    _in_out_test_type = DistanceType::GEOMETRY;
  else if (_parsed_function)
    _in_out_test_type = DistanceType::SIGN_DISTANCE;
  else
    mooseError("InterceptedElementModifier: provide exactly one geometry source, but neither "
               "'signed_dist_function' nor 'in_out_test' was set.");
}

SubdomainID
InterceptedElementModifier::computeSubdomainID()
{
  const Elem * elem = this->_current_elem;
  if (!elem)
    mooseError("InterceptedElementModifier: _current_elem is null!");

  auto check_lambda_flags = [&](Real ratio_active) -> SubdomainID
  { return isInactive(ratio_active, _lambda) ? _subdomain_id_outside : _subdomain_id_inside; };

  if (_in_out_test_type == DistanceType::SIGN_DISTANCE)
  {
    Real min_val = std::numeric_limits<Real>::max();
    Real max_val = std::numeric_limits<Real>::lowest();

    for (const auto node : make_range(elem->n_nodes()))
    {
      Real val = _parsed_function->value(_t, elem->point(node));
      min_val = std::min(min_val, val);
      max_val = std::max(max_val, val);
    }

    const bool all_nodes_active = (_is_domain_inside_surface && max_val < _threshold) ||
                                  (!_is_domain_inside_surface && min_val > _threshold);
    const bool all_nodes_inactive = (_is_domain_inside_surface && min_val > _threshold) ||
                                    (!_is_domain_inside_surface && max_val < _threshold);
    auto is_active = [&](const Point & p)
    {
      Real val = _parsed_function->value(_t, p);
      return (_is_domain_inside_surface && val < _threshold) ||
             (!_is_domain_inside_surface && val > _threshold);
    };

    const Real ratio_active = SBMUtils::activeElementFraction(*elem, _qrule_order, is_active);
    // Same-side nodes do not rule out a surface crossing the element or enclosing a region within
    // it. Quadrature sampling detects most such cases, but may miss very small regions. Exact
    // endpoint comparisons are intentional: activeElementFraction returns exactly zero or one when
    // no or all quadrature points are active, respectively.
    if (all_nodes_active && ratio_active == 1.0)
      return _subdomain_id_inside;
    if (all_nodes_inactive && ratio_active == 0.0)
      return _subdomain_id_outside;

    if (_mark_intercepted /*optional*/)
      return _subdomain_id_intercepted;

    return check_lambda_flags(ratio_active);
  }
  else if (_in_out_test_type == DistanceType::GEOMETRY)
  {
    unsigned int inside_nodes = 0;
    for (const auto node : make_range(elem->n_nodes()))
      if (_in_out_test_base->ifInside(elem->point(node)))
        ++inside_nodes;

    const unsigned int active_nodes =
        _is_domain_inside_surface ? inside_nodes : elem->n_nodes() - inside_nodes;

    auto is_active = [&](const Point & p)
    {
      return (_is_domain_inside_surface && _in_out_test_base->ifInside(p)) ||
             (!_is_domain_inside_surface && !_in_out_test_base->ifInside(p));
    };

    const Real ratio_active = SBMUtils::activeElementFraction(*elem, _qrule_order, is_active);
    // Same-side nodes do not rule out a surface crossing the element or enclosing a region within
    // it. Quadrature sampling detects most such cases, but may miss very small regions. Exact
    // endpoint comparisons are intentional: activeElementFraction returns exactly zero or one when
    // no or all quadrature points are active, respectively.
    if (active_nodes == elem->n_nodes() && ratio_active == 1.0)
      return _subdomain_id_inside;
    if (active_nodes == 0 && ratio_active == 0.0)
      return _subdomain_id_outside;

    if (_mark_intercepted /*optional*/)
      return _subdomain_id_intercepted;

    return check_lambda_flags(ratio_active);
  }
  else
  {
    mooseError("InterceptedElementModifier: Unknown AdaptiveRayContainmentCheck type!");
  }

  return -1; // fallback (shouldn't reach)
}

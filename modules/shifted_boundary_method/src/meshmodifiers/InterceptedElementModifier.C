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
#include "Function.h"
#include "PointInSurfaceCheckInterface.h"
#include "UserObjectBase.h"

registerMooseObject("ShiftedBoundaryMethodApp", InterceptedElementModifier);

InputParameters
InterceptedElementModifier::validParams()
{
  InputParameters params = SBMElementSubdomainModifierBase::validParams();

  params.addClassDescription("Marks elements as inside, outside, or intercepted based on a given "
                             "distance function or geometry.");

  params.addParam<FunctionName>(
      "signed_dist_function",
      "Signed distance function to evaluate. Exactly one of 'signed_dist_function' and "
      "'in_out_test' must be provided; providing neither or both is invalid.");

  params.addRequiredParam<SubdomainID>("subdomain_id_inside", "ID for inside elements.");
  params.addRequiredParam<SubdomainID>("subdomain_id_outside", "ID for outside elements.");

  params.addParam<Real>("threshold", 0, "Threshold for inside/outside classification.");
  params.addRequiredParam<bool>(
      "is_domain_inside_surface",
      "When true, the retained (inside) domain is the region enclosed by the surface (signed "
      "distance below 'threshold', or points reported inside by the in-out test); when false, the "
      "retained domain is the region outside the surface.");

  params.addParam<UserObjectName>(
      "in_out_test",
      "The name of the in-out test user object. Exactly one of 'in_out_test' and "
      "'signed_dist_function' must be provided; providing neither or both is invalid.");

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
    _in_out_test_base(nullptr)
{
  // Register by name before sorting and defer the lookup, as done by MeshCut2DUserObjectBase.
  if (isParamSetByUser("in_out_test"))
    _depend_uo.insert(getParam<UserObjectName>("in_out_test"));
}

const PointInSurfaceCheckInterface *
InterceptedElementModifier::getCheckedInOutTest()
{
  const UserObjectBase & base = getUserObjectBase("in_out_test");
  const auto * check = dynamic_cast<const PointInSurfaceCheckInterface *>(&base);
  if (!check)
    paramError("in_out_test",
               "'",
               base.name(),
               "' (type ",
               base.type(),
               ") does not implement the point-in-surface check interface.");
  return check;
}

/// Validate that exactly one geometry source (signed_dist_function or in_out_test) is set,
/// after running the base-class setup
void
InterceptedElementModifier::initialSetup()
{
  // Run the base class setup (reinitialize subdomains/variables, moving boundary maps,
  // reinitialization strategy, and parameter consistency checks) before our own.
  SBMElementSubdomainModifierBase::initialSetup();

  if (isParamSetByUser("in_out_test"))
    _in_out_test_base = getCheckedInOutTest();

  // Exactly one geometry source must be provided.
  if (_in_out_test_base && _parsed_function)
    mooseError("InterceptedElementModifier: provide exactly one geometry source, but both "
               "'signed_dist_function' and 'in_out_test' were set.");

  if (_in_out_test_base)
    _in_out_test_type = DistanceType::GEOMETRY;
  else if (_parsed_function)
    _in_out_test_type = DistanceType::SIGNED_DISTANCE;
  else
    mooseError("InterceptedElementModifier: provide exactly one geometry source, but neither "
               "'signed_dist_function' nor 'in_out_test' was set.");
}

SubdomainID
InterceptedElementModifier::computeSubdomainID()
{
  const Elem * elem = _current_elem;
  if (!elem)
    mooseError("InterceptedElementModifier: _current_elem is null!");

  const auto classify_from_occupancy =
      [this](const SBMUtils::ElementDomainOccupancy & occupancy) -> SubdomainID
  {
    const SBMUtils::ClassificationSubdomains subdomain_id_settings{
        _subdomain_id_inside, _subdomain_id_outside, _intercepted_subdomain_policy.subdomain_id};
    return SBMUtils::classifySubdomainFromOccupancy(
        occupancy, subdomain_id_settings, _intercepted_subdomain_policy.mark_intercepted, _lambda);
  };

  switch (_in_out_test_type)
  {
    case DistanceType::SIGNED_DISTANCE:
    {
      const auto is_in_domain = [this](const Point & point)
      {
        const Real val = _parsed_function->value(_t, point);
        return (_is_domain_inside_surface && val < _threshold) ||
               (!_is_domain_inside_surface && val > _threshold);
      };
      const auto is_outside_domain = [this](const Point & point)
      {
        const Real val = _parsed_function->value(_t, point);
        return (_is_domain_inside_surface && val > _threshold) ||
               (!_is_domain_inside_surface && val < _threshold);
      };

      return classify_from_occupancy(
          SBMUtils::elementDomainOccupancy(*elem, _qrule_order, is_in_domain, is_outside_domain));
    }

    case DistanceType::GEOMETRY:
    {
      const auto is_in_domain = [this](const Point & point)
      {
        const bool is_inside = _in_out_test_base->contains(point);
        return _is_domain_inside_surface ? is_inside : !is_inside;
      };

      return classify_from_occupancy(
          SBMUtils::elementDomainOccupancy(*elem, _qrule_order, is_in_domain));
    }

    case DistanceType::NONE:
      mooseError("InterceptedElementModifier: DistanceType::NONE is invalid in "
                 "computeSubdomainID().");
  }

  mooseError("InterceptedElementModifier: unhandled DistanceType in computeSubdomainID().");
}

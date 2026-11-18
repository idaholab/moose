//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainInterceptedGenerator.h"
#include "SBMUtils.h"

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainInterceptedGenerator);

InputParameters
SubdomainInterceptedGenerator::validParams()
{
  InputParameters params = SBMSubdomainGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<std::string>("signed_dist_function",
                                       "Parsed function expression (in terms of x, y, and z) "
                                       "defining the signed distance function to "
                                       "the interface.");

  params.addRequiredParam<SubdomainID>(
      "subdomain_id_inside", "Subdomain ID to assign to elements categorized inside the domain.");
  params.addRequiredParam<SubdomainID>(
      "subdomain_id_outside", "Subdomain ID to assign to elements categorized outside the domain.");

  params.addParam<Real>(
      "threshold", 0.0, "Level-set threshold value defining the interface boundary (default 0.0).");
  params.addRequiredParam<bool>(
      "is_domain_inside_surface",
      "Whether the inside domain corresponds to the negative signed distance region (< threshold). "
      "If true, points with signed distance < threshold are categorized as inside and > threshold "
      "as outside. If false, points with signed distance > threshold are categorized as inside.");
  params.addParam<bool>(
      "multi_geo",
      false,
      "Whether to enable multi-geometry handling when chaining generators, allowing elements with "
      "specific existing subdomain IDs to be preserved without reclassification (see "
      "'keep_inside_as_inside' and 'keep_outside_as_outside').");
  params.addParam<bool>("keep_inside_as_inside",
                        false,
                        "When 'multi_geo' is true, elements whose existing subdomain ID matches "
                        "'subdomain_id_inside' "
                        "are skipped, preserving their existing subdomain assignment.");
  params.addParam<bool>(
      "keep_outside_as_outside",
      false,
      "When 'multi_geo' is true, elements whose existing subdomain ID matches "
      "'subdomain_id_outside' are skipped, preserving their existing subdomain assignment.");

  params.addParam<bool>(
      "modify_outside_only",
      false,
      "If true, only elements categorized as outside are updated to 'subdomain_id_outside'; "
      "elements categorized as inside retain their existing subdomain IDs.");
  params.addParam<bool>(
      "modify_inside_only",
      false,
      "If true, only elements categorized as inside are updated to 'subdomain_id_inside'; elements "
      "categorized as outside retain their existing subdomain IDs.");

  params.addClassDescription("Classifies mesh elements into inside, outside, and optionally "
                             "intercepted subdomains based on an analytical signed distance "
                             "function expression.");

  return params;
}

SubdomainInterceptedGenerator::SubdomainInterceptedGenerator(const InputParameters & parameters)
  : SBMSubdomainGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _parsed_function(std::make_shared<SymFunction>()),
    _subdomain_id_inside(getParam<SubdomainID>("subdomain_id_inside")),
    _subdomain_id_outside(getParam<SubdomainID>("subdomain_id_outside")),
    _threshold(getParam<Real>("threshold")),
    _is_domain_inside_surface(getParam<bool>("is_domain_inside_surface")),
    _multi_geo(getParam<bool>("multi_geo")),
    _keep_inside_as_inside(getParam<bool>("keep_inside_as_inside")),
    _keep_outside_as_outside(getParam<bool>("keep_outside_as_outside")),
    _modify_outside_only(getParam<bool>("modify_outside_only")),
    _modify_inside_only(getParam<bool>("modify_inside_only"))
{
  setParserFeatureFlags(_parsed_function);

  if (_parsed_function->Parse(getParam<std::string>("signed_dist_function"), "x,y,z") >= 0)
    mooseError("Invalid function expression provided.");

  _func_params.resize(3);
}

std::unique_ptr<libMesh::MeshBase>
SubdomainInterceptedGenerator::generate()
{
  // Take ownership of the input mesh (already cloned by getMesh()).
  std::unique_ptr<libMesh::MeshBase> mesh = std::move(_input);

  for (const auto & elem : mesh->active_element_ptr_range() /*gen only run rank = 0*/)
  {
    // Skip elements that have already been explicitly assigned by a
    //       previous geometry in a multi-geometry workflow.
    if (_multi_geo)
    {
      if (elem->subdomain_id() == _subdomain_id_inside && _keep_inside_as_inside)
        continue;
      else if (elem->subdomain_id() == _subdomain_id_outside && _keep_outside_as_outside)
        continue;
    }

    const auto outside_id = (_modify_inside_only) ? elem->subdomain_id() : _subdomain_id_outside;
    const auto inside_id = (_modify_outside_only) ? elem->subdomain_id() : _subdomain_id_inside;

    const auto is_in_domain = [&](const Point & point)
    {
      _func_params = {point(0), point(1), point(2)};
      const Real phi = evaluate(_parsed_function);
      return (_is_domain_inside_surface && phi < _threshold) ||
             (!_is_domain_inside_surface && phi > _threshold);
    };

    const auto is_outside_domain = [&](const Point & point)
    {
      _func_params = {point(0), point(1), point(2)};
      const Real phi = evaluate(_parsed_function);
      return (_is_domain_inside_surface && phi > _threshold) ||
             (!_is_domain_inside_surface && phi < _threshold);
    };

    // Calculate the element occupancy at nodes and quadrature points according to the domain
    // policy.
    const auto occupancy =
        SBMUtils::elementDomainOccupancy(*elem, _qrule_order, is_in_domain, is_outside_domain);

    // The subdomain IDs to assign for inside, outside, and intercepted elements.
    const SBMUtils::ClassificationSubdomains subdomain_id_settings{
        inside_id, outside_id, _intercepted_subdomain_policy.subdomain_id};

    // Classify the element based on its occupancy and the intercepted-subdomain policy.
    elem->subdomain_id() = SBMUtils::classifySubdomainFromOccupancy(
        occupancy, subdomain_id_settings, _intercepted_subdomain_policy.mark_intercepted, _lambda);
  }

  // Signal that the mesh has been modified and needs preparation.
  mesh->unset_is_prepared();
  return mesh;
}

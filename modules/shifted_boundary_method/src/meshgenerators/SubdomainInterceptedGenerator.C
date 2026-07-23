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
  InputParameters params = MeshGenerator::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::string>("signed_dist_function",
                                       "Signed Distance Function to evaluate");

  params.addRequiredParam<SubdomainID>("subdomain_id_inside", "ID for inside elements.");
  params.addRequiredParam<SubdomainID>("subdomain_id_outside", "ID for outside elements.");

  params.addParam<Real>("threshold", 0.0, "Threshold for inside/outside classification.");
  params.addRequiredParam<Real>("lambda", "Lambda for false intersection classification.");
  params.addRequiredParam<bool>(
      "is_domain_inside_surface",
      "If true, the inside subdomain is the region enclosed by the surface (negative signed "
      "distance side); if false, the inside subdomain is the exterior (positive side).");
  params.addParam<bool>("multi_geo", false, "Do you have multiple geometries to do in-out test?");
  params.addParam<bool>("keep_inside_as_inside", false, "Keep inside as inside.");
  params.addParam<bool>("keep_outside_as_outside", false, "Keep outside as outside.");

  params.addParam<bool>("modify_outside_only", false, "Only modify outside elements.");
  params.addParam<bool>("modify_inside_only", false, "Only modify inside elements.");

  // Quadrature order used for active‑area estimation when an element straddles the interface
  params.addRangeCheckedParam<int>("qrule_order",
                                   9,
                                   "qrule_order >= 0 & qrule_order <= 10",
                                   "Quadrature order used to estimate the active area.");

  params.addClassDescription("Base on the signed distance function to classify elements IN and OUT "
                             "elements as different subdomains.");

  return params;
}

SubdomainInterceptedGenerator::SubdomainInterceptedGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _parsed_function(std::make_shared<SymFunction>()),
    _subdomain_id_inside(getParam<SubdomainID>("subdomain_id_inside")),
    _subdomain_id_outside(getParam<SubdomainID>("subdomain_id_outside")),
    _threshold(getParam<Real>("threshold")),
    _lambda(getParam<Real>("lambda")),
    _is_domain_inside_surface(getParam<bool>("is_domain_inside_surface")),
    _multi_geo(getParam<bool>("multi_geo")),
    _keep_inside_as_inside(getParam<bool>("keep_inside_as_inside")),
    _keep_outside_as_outside(getParam<bool>("keep_outside_as_outside")),
    _modify_outside_only(getParam<bool>("modify_outside_only")),
    _modify_inside_only(getParam<bool>("modify_inside_only")),
    _qrule_order(getParam<int>("qrule_order"))
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
    // (a) Skip elements that have already been explicitly assigned by a
    //       previous geometry in a multi‑geometry workflow.
    if (_multi_geo)
    {
      if (elem->subdomain_id() == _subdomain_id_inside && _keep_inside_as_inside)
        continue;
      else if (elem->subdomain_id() == _subdomain_id_outside && _keep_outside_as_outside)
        continue;
    }

    // (b) Evaluate the distance function at the element nodes and capture
    //       the extrema.
    Real min_phi = std::numeric_limits<Real>::max();
    Real max_phi = std::numeric_limits<Real>::lowest();

    for (unsigned int n = 0; n < elem->n_nodes(); ++n)
    {
      const Point & p = elem->point(n);
      _func_params = {p(0), p(1), p(2)};
      Real phi = evaluate(_parsed_function);
      min_phi = std::min(min_phi, phi);
      max_phi = std::max(max_phi, phi);
    }

    const auto outside_id = (_modify_inside_only) ? elem->subdomain_id() : _subdomain_id_outside;
    const auto inside_id = (_modify_outside_only) ? elem->subdomain_id() : _subdomain_id_inside;

    // (c) Trivial inside / outside if all nodes are on the same side.
    if (max_phi < _threshold)
    {
      elem->subdomain_id() = _is_domain_inside_surface ? inside_id : outside_id;
      continue;
    }
    else if (min_phi > _threshold)
    {
      elem->subdomain_id() = _is_domain_inside_surface ? outside_id : inside_id;
      continue;
    }

    // (d) Element straddles the interface – estimate the active‑area ratio
    //       with Gaussian quadrature.
    auto is_active = [&](const Point & p)
    {
      _func_params = {p(0), p(1), p(2)};
      Real phi = evaluate(_parsed_function);
      return (_is_domain_inside_surface && phi < _threshold) ||
             (!_is_domain_inside_surface && phi > _threshold);
    };

    const Real ratio_active =
        SBMUtils::activeElementFraction(*elem, static_cast<Order>(_qrule_order), is_active);

    // (e) Decide inside / outside based on the inactive fraction and _lambda.
    elem->subdomain_id() = SBMUtils::isInactive(ratio_active, _lambda) ? outside_id : inside_id;
  }

  // Signal that the mesh has been modified and needs preparation.
  mesh->set_isnt_prepared();
  return mesh;
}

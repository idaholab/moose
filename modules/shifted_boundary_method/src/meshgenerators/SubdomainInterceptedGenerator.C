//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainInterceptedGenerator.h"

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
  params.addRequiredParam<bool>("outer_boundary", "Flag for outer boundary handling.");
  params.addParam<bool>("multi_geo", false, "Do you have multiple geometries to do in-out test?");
  params.addParam<bool>("keep_inside_as_inside", false, "Keep inside as inside.");
  params.addParam<bool>("keep_outside_as_outside", false, "Keep outside as outside.");

  params.addParam<bool>("modify_outside_only", false, "Only modify outside elements.");
  params.addParam<bool>("modify_inside_only", false, "Only modify inside elements.");

  // Quadrature order used for active‑area estimation when an element straddles the interface
  params.addParam<int>("qrule_order", 9, "Quadrature order used to estimate the active area.");

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
    _outer_boundary(getParam<bool>("outer_boundary")),
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
      elem->subdomain_id() = _outer_boundary ? outside_id : inside_id;
      continue;
    }
    else if (min_phi > _threshold)
    {
      elem->subdomain_id() = _outer_boundary ? inside_id : outside_id;
      continue;
    }

    // (d) Element straddles the interface – estimate the active‑area ratio
    //       with Gaussian quadrature.
    auto is_active = [&](const Point & p)
    {
      _func_params = {p(0), p(1), p(2)};
      Real phi = evaluate(_parsed_function);
      return (_outer_boundary && phi < _threshold) || (!_outer_boundary && phi > _threshold);
    };

    Real ratio_active = computeActiveAreaRatio(elem, is_active);

    // (e) Decide false / true interception based on _lambda.
    if (_lambda == 0.0)
      elem->subdomain_id() = outside_id;
    else if (_lambda == 1.0)
      elem->subdomain_id() = inside_id;
    else
    {
      bool is_false_intercepted = ((1.0 - ratio_active) > _lambda);

      if (is_false_intercepted)
        elem->subdomain_id() = inside_id;
      else
        elem->subdomain_id() = outside_id;
    }
  }

  // Signal that the mesh has been modified and needs preparation.
  mesh->set_isnt_prepared();
  return mesh;
}

Order
SubdomainInterceptedGenerator::intToOrder(int value)
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

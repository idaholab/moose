//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubdomainElementModifier.h"

registerMooseObject("ShiftedBoundaryMethodApp", SubdomainElementModifier);

InputParameters
SubdomainElementModifier::validParams()
{
  InputParameters params = ElementSubdomainModifier::validParams();
  params.addClassDescription("Assign subdomain ID based on geometric inclusion using per-subdomain "
                             "in-out checks provided by a subdomain_id_tester.");

  params.addRequiredParam<UserObjectName>(
      "subdomain_id_tester",
      "The UserObject (PointInSubdomainCheckUO) for subdomain in/out tests.");

  params.addParam<Real>(
      "lambda", 0.5, "Threshold ratio to decide between inside and outside if partially inside.");

  params.addParam<int>("qrule_order",
                       9,
                       "Quadrature order for generating the Gauss points to do the IN-OUT test to "
                       "test whether the intercepted element belong to false intercepted element.");

  params.addParam<bool>(
      "mark_intercepted", false, "If true, mark the intercepted elements with the subdomain ID.");
  params.addParam<SubdomainID>("subdomain_id_intercepted",
                               "The subdomain ID to assign to intercepted elements.");

  return params;
}

SubdomainElementModifier::SubdomainElementModifier(const InputParameters & parameters)
  : ElementSubdomainModifier(parameters),
    _subdomain_id_tester(getUserObject<PointInSubdomainCheckUO>("subdomain_id_tester")),
    _lambda(getParam<Real>("lambda")),
    _qrule_order(getParam<int>("qrule_order")),
    _mark_intercepted(getParam<bool>("mark_intercepted")),
    _subdomain_id_intercepted(getParam<SubdomainID>("subdomain_id_intercepted"))
{
}

SubdomainID
SubdomainElementModifier::computeSubdomainID()
{
  const Elem * elem = this->_current_elem;
  if (!elem)
    mooseError("SubdomainElementModifier: _current_elem is null!");

  const auto & all_checkers = _subdomain_id_tester.getAllSubdomainCheckers();

  bool find_fully_inside = false;
  bool find_fully_outside = true;

  SubdomainID fully_inside_subdomain;
  SubdomainID best_intercepted_subdomain;
  Real max_ratio = -1.0;

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

  for (const auto & pair : all_checkers)
  {
    const auto & sub_id = pair.first;
    const auto & checker_ptr = pair.second;
    unsigned int num_inside_nodes = 0;
    for (unsigned int i = 0; i < elem->n_nodes(); ++i)
      if (checker_ptr->sideness(elem->point(i)) != SurfaceSide::OUTSIDE)
        ++num_inside_nodes;

    if (num_inside_nodes == elem->n_nodes())
    {
      fully_inside_subdomain = sub_id;
      find_fully_inside = true;
      break;
    }

    if (num_inside_nodes != 0)
      find_fully_outside = false;

    auto computeActiveAreaRatio = [&](const Elem * elem)
    {
      auto fe = initFEBase(elem);
      const auto & JxW = fe->get_JxW();
      const auto & q_points = fe->get_xyz();
      double active_area = 0, total_area = 0;

      for (unsigned int i = 0; i < q_points.size(); ++i)
      {
        if (checker_ptr->sideness(q_points[i]) != SurfaceSide::OUTSIDE)
          active_area += JxW[i];
        total_area += JxW[i];
      }

      return active_area / total_area;
    };

    Real ratio_active = computeActiveAreaRatio(elem);
    if (ratio_active > max_ratio)
    {
      max_ratio = ratio_active;
      best_intercepted_subdomain = sub_id;
    }
  }

  if (find_fully_inside)
    return fully_inside_subdomain;

  if (_mark_intercepted)
    return _subdomain_id_intercepted;
  else
    return best_intercepted_subdomain;

  // TODO: take care of fully outside case
}

Order
SubdomainElementModifier::intToOrder(int value)
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

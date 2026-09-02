//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PointInSubdomainTestAux.h"
#include "PointInSubdomainCheckUO.h"

registerMooseObject("ShiftedBoundaryMethodTestApp", PointInSubdomainTestAux);

InputParameters
PointInSubdomainTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<UserObjectName>("subdomain_checker",
                                          "PointInSubdomainCheckUO user object to query.");

  MooseEnum methods("which_subdomain if_inside");
  params.addRequiredParam<MooseEnum>(
      "method",
      methods,
      "Which PointInSubdomainCheckUO accessor to call at each element centroid.");

  params.addClassDescription("Test-only AuxKernel that exposes the PointInSubdomainCheckUO "
                             "whichSubdomain and ifInside accessors for coverage testing.");

  return params;
}

PointInSubdomainTestAux::PointInSubdomainTestAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _subdomain_checker(getUserObject<PointInSubdomainCheckUO>("subdomain_checker")),
    _method(getParam<MooseEnum>("method"))
{
  if (isNodal())
    paramError("variable", "This AuxKernel only supports Elemental fields");
}

Real
PointInSubdomainTestAux::computeValue()
{
  const Point pt = _current_elem->vertex_average();

  if (_method == "which_subdomain")
    return static_cast<Real>(_subdomain_checker.whichSubdomain(pt));

  return _subdomain_checker.ifInside(pt) ? 1.0 : 0.0;
}

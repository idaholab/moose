//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceConstraint.h"

registerMooseObject("HeatTransferApp", GapConductanceConstraint);

InputParameters
GapConductanceConstraint::validParams()
{
  InputParameters params = ModularGapConductanceConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");
  params.addRequiredParam<Real>("k", "Gap conductance");
  params.addParam<Real>("min_gap",
                        1e-6,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");

  // The flux is k/l (T^(1) - PT^(2)) with the plate gap width l, independent of the coordinate
  // system
  params.set<MooseEnum>("gap_geometry_type") = "PLATE";
  params.suppressParameter<MooseEnum>("gap_geometry_type");
  params.set<std::vector<UserObjectName>>("gap_flux_models") = {};
  params.suppressParameter<std::vector<UserObjectName>>("gap_flux_models");
  params.suppressParameter<Real>("max_gap");
  params.suppressParameter<RealVectorValue>("cylinder_axis_point_1");
  params.suppressParameter<RealVectorValue>("cylinder_axis_point_2");
  params.suppressParameter<RealVectorValue>("sphere_origin");
  return params;
}

GapConductanceConstraint::GapConductanceConstraint(const InputParameters & parameters)
  : ModularGapConductanceConstraint(parameters),
    _k(getParam<Real>("k")),
    _min_gap(getParam<Real>("min_gap"))
{
}

ADReal
GapConductanceConstraint::computeGapFlux()
{
  using std::max;
  return _k * (_u_primary[_qp] - _u_secondary[_qp]) / max(gapWidth(), _min_gap);
}

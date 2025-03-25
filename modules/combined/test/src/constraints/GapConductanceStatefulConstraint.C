//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceStatefulConstraint.h"

registerMooseObject("CombinedTestApp", GapConductanceStatefulConstraint);

InputParameters
GapConductanceStatefulConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. It pulls an auxiliary variable that acts as "
      "an old material property recovered at the node using the nodal patch recovery capability.");
  params.addRequiredParam<Real>("k", "Gap conductance.");
  params.addCoupledVar(
      "stateful_variable",
      "The history variable whose old state we use for the constraint computation.");
  params.addParam<Real>("min_gap",
                        1e-3,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");
  params.addCoupledVar("displacements", "Displacement variables.");
  return params;
}

GapConductanceStatefulConstraint::GapConductanceStatefulConstraint(
    const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _k(getParam<Real>("k")),
    _min_gap(getParam<Real>("min_gap")),
    _disp_name(parameters.getVecMooseType("displacements")),
    _n_disp(_disp_name.size()),
    _disp_secondary(_n_disp),
    _disp_primary(_n_disp),
    _stress_old(isCoupled("stateful_variable") ? coupledValueOld("stateful_variable") : _zero),
    _stress_neighbor_old(
        isCoupled("stateful_variable") ? coupledNeighborValueOld("stateful_variable") : _zero)
{
  for (unsigned int i = 0; i < _n_disp; ++i)
  {
    auto & disp_var = _subproblem.getStandardVariable(_tid, _disp_name[i]);
    _disp_secondary[i] = &disp_var.adSln();
    _disp_primary[i] = &disp_var.adSlnNeighbor();
  }
}

ADReal
GapConductanceStatefulConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  switch (mortar_type)
  {
    case Moose::MortarType::Primary:
      return _lambda[_qp] * _test_primary[_i][_qp];
    case Moose::MortarType::Secondary:
      return -_lambda[_qp] * _test_secondary[_i][_qp];
    case Moose::MortarType::Lower:
    {
      // we are creating an AD version of phys points primary and secondary here...
      ADRealVectorValue ad_phys_points_primary = _phys_points_primary[_qp];
      ADRealVectorValue ad_phys_points_secondary = _phys_points_secondary[_qp];

      // ...which uses the derivative vector of the primary and secondary displacements as
      // an approximation of the true phys points derivatives when the mesh is displacing
      if (_displaced)
        for (unsigned int i = 0; i < _n_disp; ++i)
        {
          ad_phys_points_primary(i).derivatives() = (*_disp_primary[i])[_qp].derivatives();
          ad_phys_points_secondary(i).derivatives() = (*_disp_secondary[i])[_qp].derivatives();
        }

      auto l =
          std::max((ad_phys_points_primary - ad_phys_points_secondary) * _normals[_qp], _min_gap);
      return (_lambda[_qp] - (_k + std::abs(_stress_old[_qp] + _stress_neighbor_old[_qp])) *
                                 (_u_primary[_qp] - _u_secondary[_qp]) / l) *
             _test[_i][_qp];
    }

    default:
      return 0;
  }
}

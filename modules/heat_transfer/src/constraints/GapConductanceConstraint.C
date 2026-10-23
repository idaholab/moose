//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapConductanceConstraint.h"
#include "ADUtils.h"
#include "AutomaticMortarGeneration.h"
#include "MooseVariableFE.h"

registerMooseObject("HeatTransferApp", GapConductanceConstraint);

InputParameters
GapConductanceConstraint::validParams()
{
  InputParameters params = ADMortarConstraint::validParams();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");
  params.addRequiredParam<Real>("k", "Gap conductance");
  params.addParam<Real>("min_gap",
                        1e-6,
                        "The minimum gap distance allowed. This helps with preventing the heat "
                        "flux from going to infinity as the gap approaches zero.");
  params.addCoupledVar("displacements", "Displacement variables");
  // The nodal normal at a secondary node depends on every face incident to it, so the Jacobian
  // needs the secondary face point-neighbor ghosted and coupled.
  params.set<bool>("ghost_point_neighbors") = true;
  return params;
}

GapConductanceConstraint::GapConductanceConstraint(const InputParameters & parameters)
  : ADMortarConstraint(parameters),
    _k(getParam<Real>("k")),
    _min_gap(getParam<Real>("min_gap")),
    _disp_name(parameters.getVecMooseType("displacements")),
    _n_disp(_disp_name.size()),
    _disp_secondary(_n_disp),
    _disp_primary(_n_disp),
    _nodal_frames(displacementVariables())
{
  for (unsigned int i = 0; i < _n_disp; ++i)
  {
    auto & disp_var = _subproblem.getStandardVariable(_tid, _disp_name[i]);
    _disp_secondary[i] = &disp_var.adSln();
    _disp_primary[i] = &disp_var.adSlnNeighbor();
  }
}

std::array<const MooseVariable *, 3>
GapConductanceConstraint::displacementVariables() const
{
  std::array<const MooseVariable *, 3> disp_vars{{nullptr, nullptr, nullptr}};
  for (const auto i : make_range(_n_disp))
    disp_vars[i] = &_subproblem.getStandardVariable(_tid, _disp_name[i]);
  return disp_vars;
}

void
GapConductanceConstraint::initialSetup()
{
  ADMortarConstraint::initialSetup();

  // The gap width uses quadrature-point normals interpolated from the nodal normals.
  // Displacements solved in another system or as auxiliary variables enter it with the stored
  // normals only.
  _normal_derivatives =
      _displaced && _n_disp && interpolateNormals() &&
      Moose::Mortar::supportsNodalNormalDerivatives(displacementVariables(), _sys);
}

void
GapConductanceConstraint::residualSetup()
{
  ADMortarConstraint::residualSetup();
  _nodal_frames.clear();
  _qp_normals_elem = nullptr;
}

void
GapConductanceConstraint::jacobianSetup()
{
  ADMortarConstraint::jacobianSetup();
  _nodal_frames.clear();
  _qp_normals_elem = nullptr;
}

const ADRealVectorValue &
GapConductanceConstraint::qpNormal()
{
  // Each mortar segment reinitializes the secondary face at its own quadrature points, so the
  // cache is keyed on those points rather than on the secondary element
  const auto & xi_pts = _qrule_face->get_points();
  if (_qp_normals_elem != _lower_secondary_elem || _qp_normals_xi != xi_pts)
  {
    const bool do_derivatives = _normal_derivatives && Moose::doDerivatives(_subproblem, _sys);
    _qp_normals = _nodal_frames.interpolatedNormals(
        amg(), *_lower_secondary_elem, xi_pts, _normals, do_derivatives);
    _qp_normals_elem = _lower_secondary_elem;
    _qp_normals_xi = xi_pts;
  }
  return _qp_normals[_qp];
}

ADReal
GapConductanceConstraint::computeQpResidual(Moose::MortarType mortar_type)
{
  using std::max;

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

      auto l = max((ad_phys_points_primary - ad_phys_points_secondary) * qpNormal(), _min_gap);
      return (_lambda[_qp] - _k * (_u_primary[_qp] - _u_secondary[_qp]) / l) * _test[_i][_qp];
    }

    default:
      return 0;
  }
}

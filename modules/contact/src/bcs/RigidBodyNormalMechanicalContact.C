//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidBodyNormalMechanicalContact.h"
#include "LevelSetContactor.h"
#include "MooseVariableFE.h"

registerMooseObject("ContactApp", RigidBodyNormalMechanicalContact);

InputParameters
RigidBodyNormalMechanicalContact::validParams()
{
  InputParameters params = LowerDIntegratedBC::validParams();
  params.addRequiredParam<UserObjectName>("contactor", "LevelSetContactor UO.");
  MooseEnum comp("x=0 y=1 z=2");
  params.addRequiredParam<MooseEnum>("component", comp, "Displacement component.");
  params.addRequiredCoupledVar("displacements", "Displacement components.");
  params.addParam<bool>("finite_strain", false, "Include -lambda*H_kl*phi*phi Jacobian term.");
  return params;
}

RigidBodyNormalMechanicalContact::RigidBodyNormalMechanicalContact(const InputParameters & p)
  : LowerDIntegratedBC(p),
    _contactor(getUserObject<LevelSetContactor>("contactor")),
    _component(getParam<MooseEnum>("component")),
    _ndisp(coupledComponents("displacements")),
    _disp(_ndisp),
    _disp_num(_ndisp),
    _finite_strain(getParam<bool>("finite_strain"))
{
  for (const auto k : make_range(_ndisp))
  {
    _disp[k] = &coupledValue("displacements", k);
    _disp_num[k] = coupled("displacements", k);
  }
  if (_var.number() != _disp_num[_component])
    paramError("variable", "variable must match component displacement.");
}

Point
RigidBodyNormalMechanicalContact::deformedPoint() const
{
  Point x = _q_point[_qp];
  for (const auto k : make_range(_ndisp))
    x(k) += (*_disp[k])[_qp];
  return x;
}

unsigned int
RigidBodyNormalMechanicalContact::dispIndex(unsigned int v) const
{
  for (const auto k : make_range(_ndisp))
    if (_disp_num[k] == v)
      return k;
  return libMesh::invalid_uint;
}

const LevelSetContactor::Query &
RigidBodyNormalMechanicalContact::query() const
{
  const Point pt = deformedPoint();
  if (!_cache_valid || _cache_pt != pt)
  {
    _cache_pt = pt;
    _cache_q = _contactor.queryAt(pt);
    _cache_valid = true;
  }
  return _cache_q;
}

Real
RigidBodyNormalMechanicalContact::computeQpResidual()
{
  return -_lambda[_qp] * query().normal(_component) * _test[_i][_qp];
}

Real
RigidBodyNormalMechanicalContact::computeQpJacobian()
{
  if (!_finite_strain)
    return 0;
  return -_lambda[_qp] * query().hessian(_component, _component) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
RigidBodyNormalMechanicalContact::computeQpOffDiagJacobian(unsigned int jv)
{
  if (!_finite_strain)
    return 0;
  const auto l = dispIndex(jv);
  if (l == libMesh::invalid_uint || l == _component)
    return 0;
  return -_lambda[_qp] * query().hessian(_component, l) * _phi[_j][_qp] * _test[_i][_qp];
}

Real
RigidBodyNormalMechanicalContact::computeLowerDQpJacobian(Moose::ConstraintJacobianType type)
{
  if (type == Moose::PrimaryLower)
    return -query().normal(_component) * _test[_i][_qp] * _phi_lambda[_j][_qp];
  return 0;
}

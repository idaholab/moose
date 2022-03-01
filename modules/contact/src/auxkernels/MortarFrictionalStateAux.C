//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MortarFrictionalStateAux.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", MortarFrictionalStateAux);

InputParameters
MortarFrictionalStateAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONLINEAR;
  params.addClassDescription("This class creates discrete states for nodes into frictional "
                             "contact, including contact/no-contact and stick/slip.");
  params.addRequiredCoupledVar("tangent_one",
                               "First tangent vector Lagrange multiplier for computing the mortar "
                               "frictional pressure vector.");
  params.addCoupledVar("tangent_two",
                       "Second tangent vector Lagrange multiplier for computing the mortar "
                       "frictional pressure vector.");
  params.addRequiredCoupledVar(
      "contact_pressure",
      "Normal contact pressure Lagrange multiplier from the mortar contact enforcement.");
  params.addRequiredParam<Real>("mu", "Friction coefficient to compute nodal states");
  params.addParam<Real>("tolerance", 1.0e-3, "Tolerance value used to determine the states");
  params.addParam<bool>(
      "use_displaced_mesh", true, "Whether to use the displaced mesh to get the mortar interface.");
  return params;
}

MortarFrictionalStateAux::MortarFrictionalStateAux(const InputParameters & params)
  : AuxKernel(params),
    _tangent_one(coupledValueLower("tangent_one")),
    _tangent_two(isParamValid("tangent_two") ? coupledValueLower("tangent_two") : _zero),
    _contact_pressure(coupledValueLower("contact_pressure")),
    _use_displaced_mesh(getParam<bool>("use_displaced_mesh")),
    _mu(getParam<Real>("mu")),
    _tolerance(getParam<Real>("tolerance"))
{
  // Only consider nodal quantities
  if (!isNodal())
    mooseError("MortarFrictionalStateAux auxiliary kernel can only be used with nodal kernels.");

  if (!_use_displaced_mesh)
    paramError("use_displaced_mesh",
               "This auxiliary kernel requires the use of displaced meshes to compute the "
               "frictional pressure vector.");

  // Kernel need to be boundary restricted
  if (!this->_bnd)
    paramError("boundary",
               "MortarFrictionalStateAux auxiliary kernel must be restricted to a boundary.");

  const auto mortar_dimension = _subproblem.mesh().dimension() - 1;
  if (mortar_dimension == 2 && !isParamValid("tangent_two"))
    paramError("tangent_two",
               "MortarFrictionalStateAux auxiliary kernel requires a second tangent Lagrange "
               "multiplier for three-dimensional problems");
}

Real
MortarFrictionalStateAux::computeValue()
{
  // 0-NaN: Error
  // 1: Node is not in contact
  // 2: Node is in contact and sticking
  // 3: Node is in contact and sliding

  Real status = 1;

  if (_contact_pressure[_qp] > _tolerance)
    status = 2;

  const Real tangential_pressure =
      std::sqrt(_tangent_one[_qp] * _tangent_one[_qp] + _tangent_two[_qp] * _tangent_two[_qp]);

  const Real tangential_pressure_sat = _mu * _contact_pressure[_qp];

  if (status == 2 && tangential_pressure * (1.0 + _tolerance) > tangential_pressure_sat)
    status = 3;

  return status;
}

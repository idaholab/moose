//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyBC.h"
#include "Function.h"

registerMooseObject("NavierStokesApp", MassFluxPenaltyBC);

InputParameters
MassFluxPenaltyBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addRequiredCoupledVar("v", "The y-velocity");
  params.addRequiredParam<unsigned short>("component",
                                          "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty to multiply the jump with");
  params.addClassDescription("Adds the exterior boundary contribution of penalized jumps in the "
                             "velocity variable in one component of the momentum equations.");
  params.addRequiredParam<MooseFunctorName>(
      "face_functor",
      "The velocity value on the boundary. For a Dirichlet boundary this should be the Dirichlet "
      "value. For a Neumann boundary this should be the trace velocity");
  return params;
}

MassFluxPenaltyBC::MassFluxPenaltyBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _vel_x(adCoupledValue("u")),
    _vel_y(adCoupledValue("v")),
    _comp(getParam<unsigned short>("component")),
    _matrix_only(getParam<bool>("matrix_only")),
    _gamma(getParam<Real>("gamma")),
    _face_functor(getFunctor<ADRealVectorValue>("face_functor"))
{
  if (_mesh.dimension() > 2)
    mooseError("This class only supports 2D simulations at this time");
}

void
MassFluxPenaltyBC::computeResidual()
{
  if (!_matrix_only)
    ADIntegratedBC::computeResidual();
}

ADReal
MassFluxPenaltyBC::computeQpResidual()
{
  ADRealVectorValue soln_jump(_vel_x[_qp], _vel_y[_qp], 0);
  soln_jump -=
      _face_functor(Moose::ElemSideQpArg{_current_elem, _current_side, _qp, _qrule, _q_point[_qp]},
                    determineState());

  return _gamma * soln_jump * _normals[_qp] * _test[_i][_qp] * _normals[_qp](_comp);
}

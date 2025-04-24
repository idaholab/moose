//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenalty.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("NavierStokesApp", MassFluxPenalty);

InputParameters
MassFluxPenalty::validParams()
{
  InputParameters params = HDGKernel::validParams();
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addRequiredCoupledVar("v", "The y-velocity");
  params.addRequiredParam<unsigned short>("component",
                                          "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty to multiply the jump");
  return params;
}

MassFluxPenalty::MassFluxPenalty(const InputParameters & parameters)
  : HDGKernel(parameters),
    _vel_x(adCoupledValue("u")),
    _vel_y(adCoupledValue("v")),
    _comp(getParam<unsigned short>("component")),
    _matrix_only(getParam<bool>("matrix_only")),
    _gamma(getParam<Real>("gamma"))
{
}

template <typename T>
void
MassFluxPenalty::computeOnSideHelper(std::vector<T> & residuals)
{
  residuals.resize(_test.size());
  for (auto & r : residuals)
    r = 0;

  auto return_residual = [this]() -> T
  {
    if constexpr (std::is_same<T, Real>::value)
      return MetaPhysicL::raw_value(computeQpResidualOnSide());
    else
      return computeQpResidualOnSide();
  };

  for (_qp = 0; _qp < _qrule_face->n_points(); ++_qp)
  {
    const auto jxw_c = _JxW_face[_qp] * _coord[_qp];
    for (_i = 0; _i < _test.size(); _i++)
      residuals[_i] += jxw_c * return_residual();
  }
}

void
MassFluxPenalty::computeResidualOnSide()
{
  computeOnSideHelper(_residuals);
  addResiduals(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

void
MassFluxPenalty::computeJacobianOnSide()
{
  computeOnSideHelper(_ad_residuals);
  addJacobian(_assembly, _ad_residuals, _var.dofIndices(), _var.scalingFactor());
}

ADReal
MassFluxPenalty::computeQpResidualOnSide()
{
  // Note that we name this the 'jump' because we will visit internal sides twice, once for each
  // element that shares this face. During these two different visits the normals will change
  // direction leading to a jump term
  const ADRealVectorValue soln_jump(_vel_x[_qp], _vel_y[_qp], 0);

  return _gamma * soln_jump * _normals[_qp] * _test[_i][_qp] * _normals[_qp](_comp);
}

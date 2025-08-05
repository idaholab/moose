//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxPenaltyIPHDG.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("NavierStokesApp", MassFluxPenaltyIPHDG);

InputParameters
MassFluxPenaltyIPHDG::validParams()
{
  InputParameters params = HDGKernel::validParams();
  params.addRequiredParam<NonlinearVariableName>("u", "The x-velocity");
  params.addRequiredParam<NonlinearVariableName>("v", "The y-velocity");
  params.addRequiredParam<NonlinearVariableName>("u_face", "The x-velocity on the face");
  params.addRequiredParam<NonlinearVariableName>("v_face", "The y-velocity on the face");
  params.addRequiredRangeCheckedParam<unsigned short>(
      "component", "0<=component<=1", "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty to multiply the jump");
  params.addClassDescription("introduces a jump correction on internal faces for grad-div "
                             "stabilization for discontinuous Galerkin methods. Because this is "
                             "derived from HDGKernel this class executes twice per face.");
  return params;
}

MassFluxPenaltyIPHDG::MassFluxPenaltyIPHDG(const InputParameters & parameters)
  : HDGKernel(parameters),
    _vel_x_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("u"))),
    _vel_y_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("v"))),
    _vel_x_face_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("u_face"))),
    _vel_y_face_var(_sys.getFieldVariable<Real>(_tid, getParam<NonlinearVariableName>("v_face"))),
    _vel_x(_vel_x_var.adSln()),
    _vel_y(_vel_y_var.adSln()),
    _vel_x_face(_vel_x_face_var.adSln()),
    _vel_y_face(_vel_y_face_var.adSln()),
    _vel_x_phi(_vel_x_var.phiFace()),
    _vel_y_phi(_vel_y_var.phiFace()),
    _vel_x_face_phi(_vel_x_face_var.phiFace()),
    _vel_y_face_phi(_vel_y_face_var.phiFace()),
    _comp(getParam<unsigned short>("component")),
    _gamma(getParam<Real>("gamma"))
{
  if (_mesh.dimension() > 2)
    mooseError("Only two-dimensional velocity is currently implemented");
}

template <typename T>
void
MassFluxPenaltyIPHDG::computeOnSideHelper(std::vector<T> & residuals,
                                          const MooseArray<std::vector<Real>> & test,
                                          const Real sign)
{
  residuals.resize(test.size());
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
    const auto qpres = _JxW_face[_qp] * _coord[_qp] * return_residual() * sign;
    for (const auto i : index_range(test))
      residuals[i] += qpres * test[i][_qp];
  }
}

void
MassFluxPenaltyIPHDG::computeResidualOnSide()
{
  const auto & var = _comp == 0 ? _vel_x_var : _vel_y_var;
  const auto & face_var = _comp == 0 ? _vel_x_face_var : _vel_y_face_var;
  const auto & test = _comp == 0 ? _vel_x_phi : _vel_y_phi;
  const auto & face_test = _comp == 0 ? _vel_x_face_phi : _vel_y_face_phi;
  computeOnSideHelper(_residuals, test, 1);
  addResiduals(_assembly, _residuals, var.dofIndices(), var.scalingFactor());
  computeOnSideHelper(_residuals, face_test, -1);
  addResiduals(_assembly, _residuals, face_var.dofIndices(), face_var.scalingFactor());
}

void
MassFluxPenaltyIPHDG::computeJacobianOnSide()
{
  const auto & var = _comp == 0 ? _vel_x_var : _vel_y_var;
  const auto & face_var = _comp == 0 ? _vel_x_face_var : _vel_y_face_var;
  const auto & test = _comp == 0 ? _vel_x_phi : _vel_y_phi;
  const auto & face_test = _comp == 0 ? _vel_x_face_phi : _vel_y_face_phi;
  computeOnSideHelper(_ad_residuals, test, 1);
  addJacobian(_assembly, _ad_residuals, var.dofIndices(), var.scalingFactor());
  computeOnSideHelper(_ad_residuals, face_test, -1);
  addJacobian(_assembly, _ad_residuals, face_var.dofIndices(), face_var.scalingFactor());
}

ADReal
MassFluxPenaltyIPHDG::computeQpResidualOnSide()
{
  const ADRealVectorValue soln_jump(
      _vel_x[_qp] - _vel_x_face[_qp], _vel_y[_qp] - _vel_y_face[_qp], 0);

  return _gamma * soln_jump * _normals[_qp] * _normals[_qp](_comp);
}

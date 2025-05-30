//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesLHDGKernel.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "FEProblemBase.h"

registerMooseObject("NavierStokesApp", NavierStokesLHDGKernel);

InputParameters
NavierStokesLHDGKernel::validParams()
{
  auto params = HDGKernel::validParams();
  params += NavierStokesLHDGAssemblyHelper::validParams();
  params.addParam<MooseFunctorName>(
      "body_force_x", 0, "Body force for the momentum equation in the x-direction");
  params.addParam<MooseFunctorName>(
      "body_force_y", 0, "Body force for the momentum equation in the y-direction");
  params.addParam<MooseFunctorName>(
      "body_force_z", 0, "Body force for the momentum equation in the z-direction");
  params.addParam<MooseFunctorName>(
      "pressure_mms_forcing_function",
      0,
      "A forcing function for the pressure (mass) equation for conducting MMS studies");
  params.addClassDescription("Implements the steady incompressible Navier-Stokes equations for a "
                             "hybridized discretization");
  params.renameParam("variable", "u", "The x-component of velocity");
  return params;
}

NavierStokesLHDGKernel::NavierStokesLHDGKernel(const InputParameters & parameters)
  : HDGKernel(parameters),
    NavierStokesLHDGAssemblyHelper(this, this, this, this, _fe_problem, _sys, _mesh, _tid),
    // body forces
    _body_force_x(getFunctor<Real>("body_force_x")),
    _body_force_y(getFunctor<Real>("body_force_y")),
    _body_force_z(getFunctor<Real>("body_force_z")),
    _pressure_mms_forcing_function(getFunctor<Real>("pressure_mms_forcing_function")),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _normals(_assembly.normals()),
    _current_side(_assembly.side())
{
  if (_mesh.dimension() > 2)
    mooseError("This class only supports 2D simulations at this time");

  _body_forces.push_back(&_body_force_x);
  _body_forces.push_back(&_body_force_y);
  _body_forces.push_back(&_body_force_z);
}

void
NavierStokesLHDGKernel::computeResidual()
{
  _grad_u_vel_re.resize(_qu_dof_indices.size());
  _grad_v_vel_re.resize(_qv_dof_indices.size());
  _u_vel_re.resize(_u_dof_indices.size());
  _v_vel_re.resize(_v_dof_indices.size());
  _p_re.resize(_p_dof_indices.size());
  _global_lm_re.resize(_global_lm_dof_indices ? _global_lm_dof_indices->size() : 0);

  // qu and u
  vectorVolumeResidual(_qu_sol, _u_sol, _JxW, *_qrule, _grad_u_vel_re);
  scalarVolumeResidual(
      _qu_sol, 0, _body_force_x, _JxW, *_qrule, _current_elem, _q_point, _u_vel_re);

  // qv and v
  vectorVolumeResidual(_qv_sol, _v_sol, _JxW, *_qrule, _grad_v_vel_re);
  scalarVolumeResidual(
      _qv_sol, 1, _body_force_y, _JxW, *_qrule, _current_elem, _q_point, _v_vel_re);

  // p
  pressureVolumeResidual(
      _pressure_mms_forcing_function, _JxW, *_qrule, _current_elem, _q_point, _p_re, _global_lm_re);

  addResiduals(_assembly, _grad_u_vel_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _grad_v_vel_re, _qv_dof_indices, _grad_v_var.scalingFactor());
  addResiduals(_assembly, _u_vel_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _v_vel_re, _v_dof_indices, _v_var.scalingFactor());
  addResiduals(_assembly, _p_re, _p_dof_indices, _pressure_var.scalingFactor());
  if (_global_lm_dof_indices)
    addResiduals(
        _assembly, _global_lm_re, *_global_lm_dof_indices, _enclosure_lm_var->scalingFactor());
}

void
NavierStokesLHDGKernel::computeJacobian()
{
  _grad_u_grad_u_jac.resize(_qu_dof_indices.size(), _qu_dof_indices.size());
  _grad_v_grad_v_jac.resize(_qv_dof_indices.size(), _qv_dof_indices.size());
  _grad_u_u_jac.resize(_qu_dof_indices.size(), _u_dof_indices.size());
  _grad_v_v_jac.resize(_qv_dof_indices.size(), _v_dof_indices.size());
  _u_grad_u_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _v_grad_v_jac.resize(_v_dof_indices.size(), _qv_dof_indices.size());
  _u_u_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _u_v_jac.resize(_u_dof_indices.size(), _v_dof_indices.size());
  _v_u_jac.resize(_v_dof_indices.size(), _u_dof_indices.size());
  _v_v_jac.resize(_v_dof_indices.size(), _v_dof_indices.size());
  _u_p_jac.resize(_u_dof_indices.size(), _p_dof_indices.size());
  _v_p_jac.resize(_v_dof_indices.size(), _p_dof_indices.size());
  _p_u_jac.resize(_p_dof_indices.size(), _u_dof_indices.size());
  _p_v_jac.resize(_p_dof_indices.size(), _v_dof_indices.size());
  _p_global_lm_jac.resize(_p_dof_indices.size(),
                          _global_lm_dof_indices ? _global_lm_dof_indices->size() : 0);
  _global_lm_p_jac.resize(_global_lm_dof_indices ? _global_lm_dof_indices->size() : 0,
                          _p_dof_indices.size());

  // qu and u
  vectorVolumeJacobian(_JxW, *_qrule, _grad_u_grad_u_jac, _grad_u_u_jac);
  scalarVolumeJacobian(0, _JxW, *_qrule, _u_grad_u_jac, _u_u_jac, _u_v_jac, _u_p_jac);

  // qv and v
  vectorVolumeJacobian(_JxW, *_qrule, _grad_v_grad_v_jac, _grad_v_v_jac);
  scalarVolumeJacobian(1, _JxW, *_qrule, _v_grad_v_jac, _v_u_jac, _v_v_jac, _v_p_jac);

  // p
  pressureVolumeJacobian(_JxW, *_qrule, _p_u_jac, _p_v_jac, _p_global_lm_jac, _global_lm_p_jac);

  addJacobian(
      _assembly, _grad_u_grad_u_jac, _qu_dof_indices, _qu_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _grad_v_grad_v_jac, _qv_dof_indices, _qv_dof_indices, _grad_v_var.scalingFactor());
  addJacobian(
      _assembly, _grad_u_u_jac, _qu_dof_indices, _u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(
      _assembly, _grad_v_v_jac, _qv_dof_indices, _v_dof_indices, _grad_v_var.scalingFactor());
  addJacobian(_assembly, _u_grad_u_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _v_grad_v_jac, _v_dof_indices, _qv_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _u_u_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_v_jac, _u_dof_indices, _v_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _v_u_jac, _v_dof_indices, _u_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_v_jac, _v_dof_indices, _v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _u_p_jac, _u_dof_indices, _p_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _v_p_jac, _v_dof_indices, _p_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _p_u_jac, _p_dof_indices, _u_dof_indices, _pressure_var.scalingFactor());
  addJacobian(_assembly, _p_v_jac, _p_dof_indices, _v_dof_indices, _pressure_var.scalingFactor());
  if (_global_lm_dof_indices)
  {
    addJacobian(_assembly,
                _p_global_lm_jac,
                _p_dof_indices,
                *_global_lm_dof_indices,
                _pressure_var.scalingFactor());
    addJacobian(_assembly,
                _global_lm_p_jac,
                *_global_lm_dof_indices,
                _p_dof_indices,
                _enclosure_lm_var->scalingFactor());
  }
}

void
NavierStokesLHDGKernel::computeResidualOnSide()
{
  const Elem * const neigh = _current_elem->neighbor_ptr(_current_side);

  _grad_u_vel_re.resize(_qu_dof_indices.size());
  _u_vel_re.resize(_u_dof_indices.size());
  _lm_u_vel_re.resize(_lm_u_dof_indices.size());
  _grad_v_vel_re.resize(_qv_dof_indices.size());
  _v_vel_re.resize(_v_dof_indices.size());
  _lm_v_vel_re.resize(_lm_v_dof_indices.size());
  _p_re.resize(_p_dof_indices.size());

  // qu, u, lm_u
  vectorFaceResidual(_lm_u_sol, _JxW_face, *_qrule_face, _normals, _grad_u_vel_re);
  scalarFaceResidual(_qu_sol, _u_sol, _lm_u_sol, 0, _JxW_face, *_qrule_face, _normals, _u_vel_re);
  lmFaceResidual(
      _qu_sol, _u_sol, _lm_u_sol, 0, _JxW_face, *_qrule_face, _normals, neigh, _lm_u_vel_re);

  // qv, v, lm_v
  vectorFaceResidual(_lm_v_sol, _JxW_face, *_qrule_face, _normals, _grad_v_vel_re);
  scalarFaceResidual(_qv_sol, _v_sol, _lm_v_sol, 1, _JxW_face, *_qrule_face, _normals, _v_vel_re);
  lmFaceResidual(
      _qv_sol, _v_sol, _lm_v_sol, 1, _JxW_face, *_qrule_face, _normals, neigh, _lm_v_vel_re);

  // p
  pressureFaceResidual(_JxW_face, *_qrule_face, _normals, _p_re);

  addResiduals(_assembly, _grad_u_vel_re, _qu_dof_indices, _grad_u_var.scalingFactor());
  addResiduals(_assembly, _u_vel_re, _u_dof_indices, _u_var.scalingFactor());
  addResiduals(_assembly, _lm_u_vel_re, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addResiduals(_assembly, _grad_v_vel_re, _qv_dof_indices, _grad_v_var.scalingFactor());
  addResiduals(_assembly, _v_vel_re, _v_dof_indices, _v_var.scalingFactor());
  addResiduals(_assembly, _lm_v_vel_re, _lm_v_dof_indices, _v_face_var.scalingFactor());
  addResiduals(_assembly, _p_re, _p_dof_indices, _pressure_var.scalingFactor());
}

void
NavierStokesLHDGKernel::computeJacobianOnSide()
{
  const Elem * const neigh = _current_elem->neighbor_ptr(_current_side);

  _grad_u_lm_u_jac.resize(_qu_dof_indices.size(), _lm_u_dof_indices.size());
  _u_grad_u_jac.resize(_u_dof_indices.size(), _qu_dof_indices.size());
  _u_u_jac.resize(_u_dof_indices.size(), _u_dof_indices.size());
  _u_lm_u_jac.resize(_u_dof_indices.size(), _lm_u_dof_indices.size());
  _u_lm_v_jac.resize(_u_dof_indices.size(), _lm_v_dof_indices.size());
  _u_p_jac.resize(_u_dof_indices.size(), _p_dof_indices.size());
  _lm_u_grad_u_jac.resize(_lm_u_dof_indices.size(), _qu_dof_indices.size());
  _lm_u_u_jac.resize(_lm_u_dof_indices.size(), _u_dof_indices.size());
  _lm_u_lm_u_jac.resize(_lm_u_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_u_lm_v_jac.resize(_lm_u_dof_indices.size(), _lm_v_dof_indices.size());
  _lm_u_p_jac.resize(_lm_u_dof_indices.size(), _p_dof_indices.size());
  _grad_v_lm_v_jac.resize(_qv_dof_indices.size(), _lm_v_dof_indices.size());
  _v_grad_v_jac.resize(_v_dof_indices.size(), _qv_dof_indices.size());
  _v_v_jac.resize(_v_dof_indices.size(), _v_dof_indices.size());
  _v_lm_u_jac.resize(_v_dof_indices.size(), _lm_u_dof_indices.size());
  _v_lm_v_jac.resize(_v_dof_indices.size(), _lm_v_dof_indices.size());
  _v_p_jac.resize(_v_dof_indices.size(), _p_dof_indices.size());
  _lm_v_grad_v_jac.resize(_lm_v_dof_indices.size(), _qv_dof_indices.size());
  _lm_v_v_jac.resize(_lm_v_dof_indices.size(), _v_dof_indices.size());
  _lm_v_lm_u_jac.resize(_lm_v_dof_indices.size(), _lm_u_dof_indices.size());
  _lm_v_lm_v_jac.resize(_lm_v_dof_indices.size(), _lm_v_dof_indices.size());
  _lm_v_p_jac.resize(_lm_v_dof_indices.size(), _p_dof_indices.size());
  _p_lm_u_jac.resize(_p_dof_indices.size(), _lm_u_dof_indices.size());
  _p_lm_v_jac.resize(_p_dof_indices.size(), _lm_v_dof_indices.size());

  // qu, u, lm_u
  vectorFaceJacobian(_JxW_face, *_qrule_face, _normals, _grad_u_lm_u_jac);
  scalarFaceJacobian(0,
                     _JxW_face,
                     *_qrule_face,
                     _normals,
                     _u_grad_u_jac,
                     _u_u_jac,
                     _u_lm_u_jac,
                     _u_p_jac,
                     _u_lm_u_jac,
                     _u_lm_v_jac);
  lmFaceJacobian(0,
                 _JxW_face,
                 *_qrule_face,
                 _normals,
                 neigh,
                 _lm_u_grad_u_jac,
                 _lm_u_u_jac,
                 _lm_u_lm_u_jac,
                 _lm_u_p_jac,
                 _lm_u_lm_u_jac,
                 _lm_u_lm_v_jac);

  // qv, v, lm_v
  vectorFaceJacobian(_JxW_face, *_qrule_face, _normals, _grad_v_lm_v_jac);
  scalarFaceJacobian(1,
                     _JxW_face,
                     *_qrule_face,
                     _normals,
                     _v_grad_v_jac,
                     _v_v_jac,
                     _v_lm_v_jac,
                     _v_p_jac,
                     _v_lm_u_jac,
                     _v_lm_v_jac);
  lmFaceJacobian(1,
                 _JxW_face,
                 *_qrule_face,
                 _normals,
                 neigh,
                 _lm_v_grad_v_jac,
                 _lm_v_v_jac,
                 _lm_v_lm_v_jac,
                 _lm_v_p_jac,
                 _lm_v_lm_u_jac,
                 _lm_v_lm_v_jac);

  // p
  pressureFaceJacobian(_JxW_face, *_qrule_face, _normals, _p_lm_u_jac, _p_lm_v_jac);

  addJacobian(
      _assembly, _grad_u_lm_u_jac, _qu_dof_indices, _lm_u_dof_indices, _grad_u_var.scalingFactor());
  addJacobian(_assembly, _u_grad_u_jac, _u_dof_indices, _qu_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_u_jac, _u_dof_indices, _u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_lm_u_jac, _u_dof_indices, _lm_u_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_lm_v_jac, _u_dof_indices, _lm_v_dof_indices, _u_var.scalingFactor());
  addJacobian(_assembly, _u_p_jac, _u_dof_indices, _p_dof_indices, _u_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_grad_u_jac, _lm_u_dof_indices, _qu_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_u_jac, _lm_u_dof_indices, _u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_lm_u_jac, _lm_u_dof_indices, _lm_u_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_lm_v_jac, _lm_u_dof_indices, _lm_v_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_u_p_jac, _lm_u_dof_indices, _p_dof_indices, _u_face_var.scalingFactor());
  addJacobian(
      _assembly, _grad_v_lm_v_jac, _qv_dof_indices, _lm_v_dof_indices, _grad_v_var.scalingFactor());
  addJacobian(_assembly, _v_grad_v_jac, _v_dof_indices, _qv_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_v_jac, _v_dof_indices, _v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_lm_u_jac, _v_dof_indices, _lm_u_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_lm_v_jac, _v_dof_indices, _lm_v_dof_indices, _v_var.scalingFactor());
  addJacobian(_assembly, _v_p_jac, _v_dof_indices, _p_dof_indices, _v_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_grad_v_jac, _lm_v_dof_indices, _qv_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_v_jac, _lm_v_dof_indices, _v_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_lm_u_jac, _lm_v_dof_indices, _lm_u_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_lm_v_jac, _lm_v_dof_indices, _lm_v_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _lm_v_p_jac, _lm_v_dof_indices, _p_dof_indices, _v_face_var.scalingFactor());
  addJacobian(
      _assembly, _p_lm_u_jac, _p_dof_indices, _lm_u_dof_indices, _pressure_var.scalingFactor());
  addJacobian(
      _assembly, _p_lm_v_jac, _p_dof_indices, _lm_v_dof_indices, _pressure_var.scalingFactor());
}

void
NavierStokesLHDGKernel::initialSetup()
{
  // This check must occur after FEProblemBase::init()
  checkCoupling();
}

void
NavierStokesLHDGKernel::jacobianSetup()
{
  _cached_elem = nullptr;
}

void
NavierStokesLHDGKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_cached_elem != _current_elem)
  {
    computeJacobian();
    _cached_elem = _current_elem;
  }
}

std::set<std::string>
NavierStokesLHDGKernel::additionalVariablesCovered()
{
  std::set<std::string> covered_vars = {_grad_u_var.name(),
                                        _u_face_var.name(),
                                        _v_var.name(),
                                        _grad_v_var.name(),
                                        _v_face_var.name(),
                                        _pressure_var.name()};
  if (_enclosure_lm_var)
    covered_vars.insert(_enclosure_lm_var->name());
  return covered_vars;
}

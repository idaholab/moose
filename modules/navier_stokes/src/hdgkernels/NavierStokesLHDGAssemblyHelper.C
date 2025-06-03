//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesLHDGAssemblyHelper.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"
#include "MooseMesh.h"
#include "MooseObject.h"
#include "NS.h"
#include "TransientInterface.h"

using namespace libMesh;

InputParameters
NavierStokesLHDGAssemblyHelper::validParams()
{
  auto params = DiffusionLHDGAssemblyHelper::validParams();
  params.addRequiredParam<NonlinearVariableName>(NS::pressure, "The pressure variable.");
  params.addRequiredParam<NonlinearVariableName>("v", "The y-component of velocity");
  params.addParam<NonlinearVariableName>("w", "The z-component of velocity");
  params.renameParam("gradient_variable", "grad_u", "The gradient of the x-component of velocity");
  params.addRequiredParam<NonlinearVariableName>("grad_v",
                                                 "The gradient of the y-component of velocity");
  params.addParam<NonlinearVariableName>("grad_w", "The gradient of the z-component of velocity");
  params.renameParam("face_variable", "face_u", "The x-component of the face velocity");
  params.addRequiredParam<NonlinearVariableName>("face_v", "The y-component of the face velocity");
  params.addParam<NonlinearVariableName>("face_w", "The z-component of the face velocity");
  params.addParam<NonlinearVariableName>(
      "enclosure_lm",
      "For enclosed problems like the lid driven cavity this variable can be provided to remove "
      "the pressure nullspace");
  params.renameParam("diffusivity", NS::mu, "The dynamic viscosity");
  params.addRequiredParam<Real>(NS::density, "The density");
  return params;
}

NavierStokesLHDGAssemblyHelper::NavierStokesLHDGAssemblyHelper(
    const MooseObject * const moose_obj,
    MaterialPropertyInterface * const mpi,
    MooseVariableDependencyInterface * const mvdi,
    const TransientInterface * const ti,
    const FEProblemBase & fe_problem,
    SystemBase & sys,
    const MooseMesh & mesh,
    const THREAD_ID tid)
  : DiffusionLHDGAssemblyHelper(moose_obj, mpi, mvdi, ti, fe_problem, sys, tid),
    // vars
    _v_var(sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("v"))),
    _w_var(mesh.dimension() > 2
               ? &sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("w"))
               : nullptr),
    _grad_v_var(sys.getFieldVariable<RealVectorValue>(
        tid, moose_obj->getParam<NonlinearVariableName>("grad_v"))),
    _grad_w_var(mesh.dimension() > 2
                    ? &sys.getFieldVariable<RealVectorValue>(
                          tid, moose_obj->getParam<NonlinearVariableName>("grad_w"))
                    : nullptr),
    _v_face_var(
        sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_v"))),
    _w_face_var(
        mesh.dimension() > 2
            ? &sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>("face_w"))
            : nullptr),
    _pressure_var(
        sys.getFieldVariable<Real>(tid, moose_obj->getParam<NonlinearVariableName>(NS::pressure))),
    _enclosure_lm_var(moose_obj->isParamValid("enclosure_lm")
                          ? &sys.getScalarVariable(
                                tid, moose_obj->getParam<NonlinearVariableName>("enclosure_lm"))
                          : nullptr),
    // dof indices
    _qv_dof_indices(_grad_v_var.dofIndices()),
    _v_dof_indices(_v_var.dofIndices()),
    _lm_v_dof_indices(_v_face_var.dofIndices()),
    _qw_dof_indices(_grad_w_var ? &_grad_w_var->dofIndices() : nullptr),
    _w_dof_indices(_w_var ? &_w_var->dofIndices() : nullptr),
    _lm_w_dof_indices(_w_face_var ? &_w_face_var->dofIndices() : nullptr),
    _p_dof_indices(_pressure_var.dofIndices()),
    _global_lm_dof_indices(_enclosure_lm_var ? &_enclosure_lm_var->dofIndices() : nullptr),
    // solutions
    _qv_sol(_grad_v_var.sln()),
    _v_sol(_v_var.sln()),
    _lm_v_sol(_v_face_var.sln()),
    _qw_sol(_grad_w_var ? &_grad_w_var->sln() : nullptr),
    _w_sol(_w_var ? &_w_var->sln() : nullptr),
    _lm_w_sol(_w_face_var ? &_w_face_var->sln() : nullptr),
    _p_sol(_pressure_var.sln()),
    _global_lm_dof_value(_enclosure_lm_var ? &_enclosure_lm_var->sln() : nullptr),
    _rho(moose_obj->getParam<Real>(NS::density))
{
  if (mesh.dimension() > 2)
    mooseError("3D not yet implemented");

  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_v_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<RealVectorValue> &>(_grad_v_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_v_face_var));
  mvdi->addMooseVariableDependency(&const_cast<MooseVariableFE<Real> &>(_pressure_var));
}

void
NavierStokesLHDGAssemblyHelper::scalarVolumeResidual(const MooseArray<Gradient> & vel_gradient,
                                                     const unsigned int vel_component,
                                                     const Moose::Functor<Real> & body_force,
                                                     const MooseArray<Real> & JxW,
                                                     const QBase & qrule,
                                                     const Elem * const current_elem,
                                                     const MooseArray<Point> & q_point,
                                                     DenseVector<Number> & scalar_re)
{
  DiffusionLHDGAssemblyHelper::scalarVolumeResidual(
      vel_gradient, body_force, JxW, qrule, current_elem, q_point, scalar_re);

  for (const auto qp : make_range(qrule.n_points()))
  {
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_u_sol, _v_sol, qp, vel_component);
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];

    for (const auto i : index_range(scalar_re))
    {
      // Scalar equation dependence on pressure dofs
      scalar_re(i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * qp_p);

      // Scalar equation dependence on scalar dofs
      scalar_re(i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
    }
  }
}

void
NavierStokesLHDGAssemblyHelper::scalarVolumeJacobian(const unsigned int vel_component,
                                                     const MooseArray<Real> & JxW,
                                                     const QBase & qrule,
                                                     DenseMatrix<Number> & scalar_vector_jac,
                                                     DenseMatrix<Number> & scalar_u_vel_jac,
                                                     DenseMatrix<Number> & scalar_v_vel_jac,
                                                     DenseMatrix<Number> & scalar_p_jac)
{
  DiffusionLHDGAssemblyHelper::scalarVolumeJacobian(JxW, qrule, scalar_vector_jac);

  for (const auto i : make_range(scalar_vector_jac.m()))
    for (const auto qp : make_range(qrule.n_points()))
    {
      // Scalar equation dependence on pressure dofs
      for (const auto j : make_range(scalar_p_jac.n()))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi[j][qp];
        scalar_p_jac(i, j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * p_phi);
      }

      // Scalar equation dependence on scalar dofs
      mooseAssert(scalar_u_vel_jac.n() == scalar_v_vel_jac.n(), "These must be the same size");
      for (const auto j : make_range(scalar_u_vel_jac.n()))
      {
        // derivatives wrt 0th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 0, _scalar_phi, j);
          scalar_u_vel_jac(i, j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
        }
        // derivatives wrt 1th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_u_sol, _v_sol, qp, vel_component, 1, _scalar_phi, j);
          scalar_v_vel_jac(i, j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * rho_vel_cross_vel);
        }
      }
    }
}

void
NavierStokesLHDGAssemblyHelper::pressureVolumeResidual(
    const Moose::Functor<Real> & pressure_mms_forcing_function,
    const MooseArray<Real> & JxW,
    const QBase & qrule,
    const Elem * const current_elem,
    const MooseArray<Point> & q_point,
    DenseVector<Number> & pressure_re,
    DenseVector<Number> & global_lm_re)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    // Prepare forcing function
    const auto f = pressure_mms_forcing_function(
        Moose::ElemQpArg{current_elem, qp, &qrule, q_point[qp]}, _ti.determineState());

    const Gradient vel(_u_sol[qp], _v_sol[qp]);
    for (const auto i : make_range(pressure_re.size()))
    {
      pressure_re(i) -= JxW[qp] * (_grad_scalar_phi[i][qp] * vel);

      // Pressure equation forcing function RHS
      pressure_re(i) -= JxW[qp] * _scalar_phi[i][qp] * f;

      if (_enclosure_lm_var)
      {
        mooseAssert(
            _global_lm_dof_value->size() == 1,
            "There should only be one degree of freedom for removing the pressure nullspace");
        pressure_re(i) -= JxW[qp] * _scalar_phi[i][qp] * (*_global_lm_dof_value)[0];
      }
    }

    if (_enclosure_lm_var)
      global_lm_re(0) -= JxW[qp] * _p_sol[qp];
  }
}

void
NavierStokesLHDGAssemblyHelper::pressureVolumeJacobian(const MooseArray<Real> & JxW,
                                                       const QBase & qrule,
                                                       DenseMatrix<Number> & p_u_vel_jac,
                                                       DenseMatrix<Number> & p_v_vel_jac,
                                                       DenseMatrix<Number> & p_global_lm_jac,
                                                       DenseMatrix<Number> & global_lm_p_jac)
{
  for (const auto qp : make_range(qrule.n_points()))
  {
    for (const auto i : make_range(p_u_vel_jac.m()))
    {
      for (const auto j : make_range(p_u_vel_jac.n()))
      {
        {
          const Gradient phi(_scalar_phi[j][qp], 0);
          p_u_vel_jac(i, j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
        {
          const Gradient phi(0, _scalar_phi[j][qp]);
          p_v_vel_jac(i, j) -= JxW[qp] * (_grad_scalar_phi[i][qp] * phi);
        }
      }
      if (_enclosure_lm_var)
        p_global_lm_jac(i, 0) -= JxW[qp] * _scalar_phi[i][qp];
    }

    if (_enclosure_lm_var)
    {
      for (const auto j : make_range(global_lm_p_jac.n()))
        global_lm_p_jac(0, j) -= JxW[qp] * _scalar_phi[j][qp];
    }
  }
}

RealVectorValue
NavierStokesLHDGAssemblyHelper::rhoVelCrossVelResidual(const MooseArray<Number> & u_sol,
                                                       const MooseArray<Number> & v_sol,
                                                       const unsigned int qp,
                                                       const unsigned int vel_component)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  return _rho * U * U(vel_component);
}

RealVectorValue
NavierStokesLHDGAssemblyHelper::rhoVelCrossVelJacobian(const MooseArray<Number> & u_sol,
                                                       const MooseArray<Number> & v_sol,
                                                       const unsigned int qp,
                                                       const unsigned int vel_component,
                                                       const unsigned int vel_j_component,
                                                       const MooseArray<std::vector<Real>> & phi,
                                                       const unsigned int j)
{
  const RealVectorValue U(u_sol[qp], v_sol[qp]);
  RealVectorValue vector_phi;
  vector_phi(vel_j_component) = phi[j][qp];
  auto ret = vector_phi * U(vel_component);
  if (vel_component == vel_j_component)
    ret += U * phi[j][qp];
  ret *= _rho;
  return ret;
}

void
NavierStokesLHDGAssemblyHelper::pressureFaceResidual(const MooseArray<Real> & JxW_face,
                                                     const QBase & qrule_face,
                                                     const MooseArray<Point> & normals,
                                                     DenseVector<Number> & pressure_re)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const Gradient vel(_lm_u_sol[qp], _lm_v_sol[qp]);
    const auto vdotn = vel * normals[qp];
    for (const auto i : make_range(pressure_re.size()))
      pressure_re(i) += JxW_face[qp] * vdotn * _scalar_phi_face[i][qp];
  }
}

void
NavierStokesLHDGAssemblyHelper::pressureFaceJacobian(const MooseArray<Real> & JxW_face,
                                                     const QBase & qrule_face,
                                                     const MooseArray<Point> & normals,
                                                     DenseMatrix<Number> & p_lm_u_vel_jac,
                                                     DenseMatrix<Number> & p_lm_v_vel_jac)
{
  for (const auto i : make_range(p_lm_u_vel_jac.m()))
    for (const auto j : make_range(p_lm_u_vel_jac.n()))
      for (const auto qp : make_range(qrule_face.n_points()))
      {
        {
          const Gradient phi(_lm_phi_face[j][qp], 0);
          p_lm_u_vel_jac(i, j) += JxW_face[qp] * phi * normals[qp] * _scalar_phi_face[i][qp];
        }
        {
          const Gradient phi(0, _lm_phi_face[j][qp]);
          p_lm_v_vel_jac(i, j) += JxW_face[qp] * phi * normals[qp] * _scalar_phi_face[i][qp];
        }
      }
}

void
NavierStokesLHDGAssemblyHelper::scalarFaceResidual(const MooseArray<Gradient> & vector_sol,
                                                   const MooseArray<Number> & scalar_sol,
                                                   const MooseArray<Number> & lm_sol,
                                                   const unsigned int vel_component,
                                                   const MooseArray<Real> & JxW_face,
                                                   const QBase & qrule_face,
                                                   const MooseArray<Point> & normals,
                                                   DenseVector<Number> & scalar_re)
{
  DiffusionLHDGAssemblyHelper::scalarFaceResidual(
      vector_sol, scalar_sol, lm_sol, JxW_face, qrule_face, normals, scalar_re);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_lm_u_sol, _lm_v_sol, qp, vel_component);

    for (const auto i : make_range(scalar_re.size()))
    {
      // pressure
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] * (qp_p * normals[qp]);

      // lm from convection term
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
    }
  }
}

void
NavierStokesLHDGAssemblyHelper::scalarFaceJacobian(const unsigned int vel_component,
                                                   const MooseArray<Real> & JxW_face,
                                                   const QBase & qrule_face,
                                                   const MooseArray<Point> & normals,
                                                   DenseMatrix<Number> & scalar_vector_jac,
                                                   DenseMatrix<Number> & scalar_scalar_jac,
                                                   DenseMatrix<Number> & scalar_lm_jac,
                                                   DenseMatrix<Number> & scalar_p_jac,
                                                   DenseMatrix<Number> & scalar_lm_u_vel_jac,
                                                   DenseMatrix<Number> & scalar_lm_v_vel_jac)

{
  DiffusionLHDGAssemblyHelper::scalarFaceJacobian(
      JxW_face, qrule_face, normals, scalar_vector_jac, scalar_scalar_jac, scalar_lm_jac);

  for (const auto i : make_range(scalar_lm_u_vel_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(scalar_p_jac.n()))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        // pressure
        scalar_p_jac(i, j) += JxW_face[qp] * _scalar_phi_face[i][qp] * (p_phi * normals[qp]);
      }

      for (const auto j : make_range(scalar_lm_u_vel_jac.n()))
      {
        //
        // from convection term
        //

        // derivatives wrt 0th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 0, _lm_phi_face, j);
          scalar_lm_u_vel_jac(i, j) +=
              JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
        }
        // derivatives wrt 1th component of velocity
        {
          const auto rho_vel_cross_vel =
              rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 1, _lm_phi_face, j);
          scalar_lm_v_vel_jac(i, j) +=
              JxW_face[qp] * _scalar_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
        }
      }
    }
}

void
NavierStokesLHDGAssemblyHelper::lmFaceResidual(const MooseArray<Gradient> & vector_sol,
                                               const MooseArray<Number> & scalar_sol,
                                               const MooseArray<Number> & lm_sol,
                                               const unsigned int vel_component,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals,
                                               const Elem * const neigh,
                                               DenseVector<Number> & lm_re)
{
  DiffusionLHDGAssemblyHelper::lmFaceResidual(
      vector_sol, scalar_sol, lm_sol, JxW_face, qrule_face, normals, lm_re);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];
    const auto rho_vel_cross_vel = rhoVelCrossVelResidual(_lm_u_sol, _lm_v_sol, qp, vel_component);

    for (const auto i : make_range(lm_re.size()))
    {
      // pressure
      lm_re(i) += JxW_face[qp] * _lm_phi_face[i][qp] * (qp_p * normals[qp]);

      // If we are an internal face we add the convective term. On the outflow boundary we do not
      // zero out the convection term, e.g. we are going to set q + p + tau * (u - u_hat) to zero
      if (neigh)
        // lm from convection term
        lm_re(i) += JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
    }
  }
}

void
NavierStokesLHDGAssemblyHelper::lmFaceJacobian(const unsigned int vel_component,
                                               const MooseArray<Real> & JxW_face,
                                               const QBase & qrule_face,
                                               const MooseArray<Point> & normals,
                                               const Elem * const neigh,
                                               DenseMatrix<Number> & lm_vec_jac,
                                               DenseMatrix<Number> & lm_scalar_jac,
                                               DenseMatrix<Number> & lm_lm_jac,
                                               DenseMatrix<Number> & lm_p_jac,
                                               DenseMatrix<Number> & lm_lm_u_vel_jac,
                                               DenseMatrix<Number> & lm_lm_v_vel_jac)
{
  DiffusionLHDGAssemblyHelper::lmFaceJacobian(
      JxW_face, qrule_face, normals, lm_vec_jac, lm_scalar_jac, lm_lm_jac);

  for (const auto i : make_range(lm_p_jac.m()))
    for (const auto qp : make_range(qrule_face.n_points()))
    {
      for (const auto j : make_range(lm_p_jac.n()))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        lm_p_jac(i, j) += JxW_face[qp] * _lm_phi_face[i][qp] * (p_phi * normals[qp]);
      }

      for (const auto j : make_range(lm_lm_u_vel_jac.n()))
        if (neigh)
        {
          // derivatives wrt 0th component of velocity
          {
            const auto rho_vel_cross_vel =
                rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 0, _lm_phi_face, j);
            lm_lm_u_vel_jac(i, j) +=
                JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
          }
          // derivatives wrt 1th component of velocity
          {
            const auto rho_vel_cross_vel =
                rhoVelCrossVelJacobian(_lm_u_sol, _lm_v_sol, qp, vel_component, 1, _lm_phi_face, j);
            lm_lm_v_vel_jac(i, j) +=
                JxW_face[qp] * _lm_phi_face[i][qp] * rho_vel_cross_vel * normals[qp];
          }
        }
    }
}

void
NavierStokesLHDGAssemblyHelper::pressureDirichletResidual(
    const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const Elem * const current_elem,
    const unsigned int current_side,
    const MooseArray<Point> & q_point_face,
    DenseVector<Number> & pressure_re)
{
  for (const auto qp : make_range(qrule_face.n_points()))
  {
    const Moose::ElemSideQpArg elem_side_qp_arg{
        current_elem, current_side, qp, &qrule_face, q_point_face[qp]};
    const auto time_arg = _ti.determineState();
    const RealVectorValue dirichlet_velocity((*dirichlet_vel[0])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[1])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[2])(elem_side_qp_arg, time_arg));
    const auto vdotn = dirichlet_velocity * normals[qp];
    for (const auto i : make_range(pressure_re.size()))
      pressure_re(i) += JxW_face[qp] * vdotn * _scalar_phi_face[i][qp];
  }
}

void
NavierStokesLHDGAssemblyHelper::scalarDirichletResidual(
    const MooseArray<Gradient> & vector_sol,
    const MooseArray<Number> & scalar_sol,
    const unsigned int vel_component,
    const std::array<const Moose::Functor<Real> *, 3> & dirichlet_vel,
    const MooseArray<Real> & JxW_face,
    const QBase & qrule_face,
    const MooseArray<Point> & normals,
    const Elem * const current_elem,
    const unsigned int current_side,
    const MooseArray<Point> & q_point_face,
    DenseVector<Number> & scalar_re)
{
  DiffusionLHDGAssemblyHelper::scalarDirichletResidual(vector_sol,
                                                       scalar_sol,
                                                       *dirichlet_vel[vel_component],
                                                       JxW_face,
                                                       qrule_face,
                                                       normals,
                                                       current_elem,
                                                       current_side,
                                                       q_point_face,
                                                       scalar_re);

  for (const auto qp : make_range(qrule_face.n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = _p_sol[qp];

    const Moose::ElemSideQpArg elem_side_qp_arg{
        current_elem, current_side, qp, &qrule_face, q_point_face[qp]};
    const auto time_arg = _ti.determineState();
    const RealVectorValue dirichlet_velocity((*dirichlet_vel[0])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[1])(elem_side_qp_arg, time_arg),
                                             (*dirichlet_vel[2])(elem_side_qp_arg, time_arg));
    const auto scalar_value = dirichlet_velocity(vel_component);

    for (const auto i : make_range(scalar_re.size()))
    {
      // pressure
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] * (qp_p * normals[qp]);

      // dirichlet lm from advection term
      scalar_re(i) += JxW_face[qp] * _scalar_phi_face[i][qp] *
                      (_rho * dirichlet_velocity * normals[qp]) * scalar_value;
    }
  }
}

void
NavierStokesLHDGAssemblyHelper::scalarDirichletJacobian(const unsigned int vel_component,
                                                        const MooseArray<Real> & JxW_face,
                                                        const QBase & qrule_face,
                                                        const MooseArray<Point> & normals,
                                                        DenseMatrix<Number> & scalar_vector_jac,
                                                        DenseMatrix<Number> & scalar_scalar_jac,
                                                        DenseMatrix<Number> & scalar_pressure_jac)
{
  DiffusionLHDGAssemblyHelper::scalarDirichletJacobian(
      JxW_face, qrule_face, normals, scalar_vector_jac, scalar_scalar_jac);

  for (const auto i : make_range(scalar_pressure_jac.m()))
    for (const auto j : make_range(scalar_pressure_jac.n()))
      for (const auto qp : make_range(qrule_face.n_points()))
      {
        Gradient p_phi;
        p_phi(vel_component) = _scalar_phi_face[j][qp];
        // pressure
        scalar_pressure_jac(i, j) += JxW_face[qp] * _scalar_phi_face[i][qp] * (p_phi * normals[qp]);
      }
}

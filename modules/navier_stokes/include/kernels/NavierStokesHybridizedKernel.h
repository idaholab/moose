//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HybridizedKernel.h"
#include "NavierStokesHybridizedInterface.h"

#include <vector>

class Function;

class NavierStokesHybridizedKernel : public HybridizedKernel, public NavierStokesHybridizedInterface
{
public:
  static InputParameters validParams();

  NavierStokesHybridizedKernel(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onElement() override;
  virtual void onInternalSide() override;

  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  static RealVectorValue velCrossVelResidual(const MooseArray<Number> & u_sol,
                                             const MooseArray<Number> & v_sol,
                                             const unsigned int qp,
                                             const unsigned int vel_component);

  /**
   * @param u_sol The x-velocity solution, can correspond to either the volumetric or face velocity
   * @param v_sol The y-velocity solution, can correspond to either the volumetric or face velocity
   */
  static RealVectorValue velCrossVelJacobian(const MooseArray<Number> & u_sol,
                                             const MooseArray<Number> & v_sol,
                                             const unsigned int qp,
                                             const unsigned int vel_component,
                                             const unsigned int vel_j_component,
                                             const MooseArray<std::vector<Real>> & phi,
                                             const unsigned int j);

  void computeStress(const MooseArray<Gradient> & vel_gradient,
                     const unsigned int vel_component,
                     std::vector<Gradient> & sigma);

  void vectorVolumeResidual(const unsigned int i_offset,
                            const MooseArray<Gradient> & vector_sol,
                            const MooseArray<Number> & scalar_sol);

  void vectorVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vector_j_offset,
                            const unsigned int scalar_j_offset);

  void scalarVolumeResidual(const unsigned int i_offset,
                            const MooseArray<Gradient> & vel_gradient,
                            const unsigned int vel_component,
                            std::vector<Gradient> & sigma);

  void scalarVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vel_gradient_j_offset,
                            const unsigned int p_j_offset,
                            const unsigned int vel_component,
                            const unsigned int u_j_offset,
                            const unsigned int v_j_offset);

  void pressureVolumeResidual(const unsigned int i_offset, const unsigned int global_lm_i_offset);

  void pressureVolumeJacobian(const unsigned int i_offset,
                              const unsigned int u_j_offset,
                              const unsigned int v_j_offset,
                              const unsigned int p_j_offset,
                              const unsigned int global_lm_offset);

  //
  // Methods which are leveraged both on internal sides in the kernel and by the outflow bc
  //

  template <typename NSHybridized>
  static void pressureFaceResidual(NSHybridized & obj, const unsigned int i_offset);

  template <typename NSHybridized>
  static void pressureFaceJacobian(NSHybridized & obj,
                                   const unsigned int i_offset,
                                   const unsigned int lm_u_j_offset,
                                   const unsigned int lm_v_j_offset);

  template <typename NSHybridized>
  static void vectorFaceResidual(NSHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Number> & lm_sol);

  template <typename NSHybridized>
  static void vectorFaceJacobian(NSHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int lm_j_offset);

  template <typename NSHybridized>
  static void scalarFaceResidual(NSHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol,
                                 const unsigned int vel_component);

  template <typename NSHybridized>
  static void scalarFaceJacobian(NSHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int vector_j_offset,
                                 const unsigned int scalar_j_offset,
                                 const unsigned int lm_j_offset,
                                 const unsigned int p_j_offset,
                                 const unsigned int vel_component,
                                 const unsigned int lm_u_j_offset,
                                 const unsigned int lm_v_j_offset);

  template <typename NSHybridized>
  static void lmFaceResidual(NSHybridized & obj,
                             const unsigned int i_offset,
                             const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol,
                             const unsigned int vel_component);

  template <typename NSHybridized>
  static void lmFaceJacobian(NSHybridized & obj,
                             const unsigned int i_offset,
                             const unsigned int vector_j_offset,
                             const unsigned int scalar_j_offset,
                             const unsigned int lm_j_offset,
                             const unsigned int p_j_offset,
                             const unsigned int vel_component,
                             const unsigned int lm_u_j_offset,
                             const unsigned int lm_v_j_offset);

  // stresses at quadrature points
  std::vector<Gradient> _sigma_u;
  std::vector<Gradient> _sigma_v;
  std::vector<Gradient> _sigma_w;

  // body forces
  const Function & _body_force_x;
  const Function & _body_force_y;
  const Function & _body_force_z;
  std::vector<const Function *> _body_forces;
  const Function & _pressure_mms_forcing_function;

  friend class NavierStokesHybridizedOutflowBC;
  friend class NavierStokesHybridizedInterface;
};

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::pressureFaceResidual(NSHybridized & obj, const unsigned int i_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    const Gradient vel(obj._lm_u_sol[qp], obj._lm_v_sol[qp]);
    const auto vdotn = vel * obj._normals[qp];
    for (const auto i : make_range(obj._p_n_dofs))
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * vdotn * obj._scalar_phi_face[i][qp];
  }
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::pressureFaceJacobian(NSHybridized & obj,
                                                   const unsigned int i_offset,
                                                   const unsigned int lm_u_j_offset,
                                                   const unsigned int lm_v_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._p_n_dofs))
      for (const auto j : make_range(obj._lm_n_dofs))
      {
        {
          const Gradient phi(obj._lm_phi_face[j][qp], 0);
          obj._LMMat(i_offset + i, lm_u_j_offset + j) +=
              obj._JxW_face[qp] * phi * obj._normals[qp] * obj._scalar_phi_face[i][qp];
        }
        {
          const Gradient phi(0, obj._lm_phi_face[j][qp]);
          obj._LMMat(i_offset + i, lm_v_j_offset + j) +=
              obj._JxW_face[qp] * phi * obj._normals[qp] * obj._scalar_phi_face[i][qp];
        }
      }
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::vectorFaceResidual(NSHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      obj._PrimalVec(i_offset + i) -=
          obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) * lm_sol[qp];
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::vectorFaceJacobian(NSHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      for (const auto j : make_range(obj._lm_n_dofs))
        obj._PrimalLM(i_offset + i, lm_j_offset + j) -=
            obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) *
            obj._lm_phi_face[j][qp];
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::scalarFaceResidual(NSHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const MooseArray<Gradient> & vector_sol,
                                                 const MooseArray<Number> & scalar_sol,
                                                 const MooseArray<Number> & lm_sol,
                                                 const unsigned int vel_component)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];
    const auto vel_cross_vel = velCrossVelResidual(obj._lm_u_sol, obj._lm_v_sol, qp, vel_component);

    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      if (obj._neigh)
      {
        // vector
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._nu[qp] *
                                        obj._scalar_phi_face[i][qp] *
                                        (vector_sol[qp] * obj._normals[qp]);

        // pressure
        obj._PrimalVec(i_offset + i) +=
            obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (qp_p * obj._normals[qp]);

        // scalar from stabilization term
        obj._PrimalVec(i_offset + i) += obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

        // lm from stabilization term
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        lm_sol[qp] * obj._normals[qp] * obj._normals[qp];
      }

      // lm from convection term
      obj._PrimalVec(i_offset + i) +=
          obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
    }
  }
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::scalarFaceJacobian(NSHybridized & obj,
                                                 const unsigned int i_offset,
                                                 const unsigned int vector_j_offset,
                                                 const unsigned int scalar_j_offset,
                                                 const unsigned int lm_j_offset,
                                                 const unsigned int p_j_offset,
                                                 const unsigned int vel_component,
                                                 const unsigned int lm_u_j_offset,
                                                 const unsigned int lm_v_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
    {
      if (obj._neigh)
      {
        for (const auto j : make_range(obj._vector_n_dofs))
          obj._PrimalMat(i_offset + i, vector_j_offset + j) -=
              obj._JxW_face[qp] * obj._nu[qp] * obj._scalar_phi_face[i][qp] *
              (obj._vector_phi_face[j][qp] * obj._normals[qp]);

        for (const auto j : make_range(obj._p_n_dofs))
        {
          Gradient p_phi;
          p_phi(vel_component) = obj._scalar_phi_face[j][qp];
          // pressure
          obj._PrimalLM(i_offset + i, p_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * (p_phi * obj._normals[qp]);
        }

        for (const auto j : make_range(obj._scalar_n_dofs))
          obj._PrimalMat(i_offset + i, scalar_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];
      }

      for (const auto j : make_range(obj._lm_n_dofs))
      {
        if (obj._neigh)
          // from stabilization term
          obj._PrimalLM(i_offset + i, lm_j_offset + j) -=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._lm_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];

        //
        // from convection term
        //

        // derivatives wrt 0th component
        {
          const auto vel_cross_vel = velCrossVelJacobian(
              obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 0, obj._lm_phi_face, j);
          obj._PrimalLM(i_offset + i, lm_u_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
        }
        // derivatives wrt 1th component
        {
          const auto vel_cross_vel = velCrossVelJacobian(
              obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 1, obj._lm_phi_face, j);
          obj._PrimalLM(i_offset + i, lm_v_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
        }
      }
    }
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::lmFaceResidual(NSHybridized & obj,
                                             const unsigned int i_offset,
                                             const MooseArray<Gradient> & vector_sol,
                                             const MooseArray<Number> & scalar_sol,
                                             const MooseArray<Number> & lm_sol,
                                             const unsigned int vel_component)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
  {
    Gradient qp_p;
    qp_p(vel_component) = obj._p_sol[qp];
    const auto vel_cross_vel = velCrossVelResidual(obj._lm_u_sol, obj._lm_v_sol, qp, vel_component);

    for (const auto i : make_range(obj._lm_n_dofs))
    {
      // vector
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._nu[qp] * obj._lm_phi_face[i][qp] *
                                  (vector_sol[qp] * obj._normals[qp]);

      // pressure
      obj._LMVec(i_offset + i) +=
          obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * (qp_p * obj._normals[qp]);

      // scalar from stabilization term
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau *
                                  scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

      // lm from stabilization term
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * lm_sol[qp] *
                                  obj._normals[qp] * obj._normals[qp];

      // If we are an internal face we add the convective term. On the outflow boundary we do not
      // zero out the convection term, e.g. we are going to set q + p + tau * (u - u_hat) to zero
      if (obj._neigh)
        // lm from convection term
        obj._LMVec(i_offset + i) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
    }
  }
}

template <typename NSHybridized>
void
NavierStokesHybridizedKernel::lmFaceJacobian(NSHybridized & obj,
                                             const unsigned int i_offset,
                                             const unsigned int vector_j_offset,
                                             const unsigned int scalar_j_offset,
                                             const unsigned int lm_j_offset,
                                             const unsigned int p_j_offset,
                                             const unsigned int vel_component,
                                             const unsigned int lm_u_j_offset,
                                             const unsigned int lm_v_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._LMPrimal(i_offset + i, vector_j_offset + j) -=
            obj._JxW_face[qp] * obj._nu[qp] * obj._lm_phi_face[i][qp] *
            (obj._vector_phi_face[j][qp] * obj._normals[qp]);

      for (const auto j : make_range(obj._p_n_dofs))
      {
        Gradient p_phi;
        p_phi(vel_component) = obj._scalar_phi_face[j][qp];
        obj._LMMat(i_offset + i, p_j_offset + j) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * (p_phi * obj._normals[qp]);
      }

      for (const auto j : make_range(obj._scalar_n_dofs))
        obj._LMPrimal(i_offset + i, scalar_j_offset + j) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
            obj._normals[qp] * obj._normals[qp];

      for (const auto j : make_range(obj._lm_n_dofs))
      {
        // from stabilization term
        obj._LMMat(i_offset + i, lm_j_offset + j) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] *
                                                     _tau * obj._lm_phi_face[j][qp] *
                                                     obj._normals[qp] * obj._normals[qp];
        if (obj._neigh)
        {
          // derivatives wrt 0th component
          {
            const auto vel_cross_vel = velCrossVelJacobian(
                obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 0, obj._lm_phi_face, j);
            obj._LMMat(i_offset + i, lm_u_j_offset + j) +=
                obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
          }
          // derivatives wrt 1th component
          {
            const auto vel_cross_vel = velCrossVelJacobian(
                obj._lm_u_sol, obj._lm_v_sol, qp, vel_component, 1, obj._lm_phi_face, j);
            obj._LMMat(i_offset + i, lm_v_j_offset + j) +=
                obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * vel_cross_vel * obj._normals[qp];
          }
        }
      }
    }
}

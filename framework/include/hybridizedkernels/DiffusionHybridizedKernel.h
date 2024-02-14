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
#include "DiffusionHybridizedInterface.h"

#include <vector>

class Function;

/**
 * Implements the diffusion equation for a hybridized discretization
 */
class DiffusionHybridizedKernel : public HybridizedKernel, public DiffusionHybridizedInterface
{
public:
  static InputParameters validParams();

  DiffusionHybridizedKernel(const InputParameters & parameters);

  virtual const MooseVariableBase & variable() const override { return _u_face_var; }

protected:
  virtual void onElement() override;
  virtual void onInternalSide() override;

  void vectorVolumeResidual(const unsigned int i_offset,
                            const MooseArray<Gradient> & vector_sol,
                            const MooseArray<Number> & scalar_sol);

  void vectorVolumeJacobian(const unsigned int i_offset,
                            const unsigned int vector_j_offset,
                            const unsigned int scalar_j_offset);

  void scalarVolumeResidual(const unsigned int i_offset, const MooseArray<Gradient> & vector_field);

  void scalarVolumeJacobian(const unsigned int i_offset, const unsigned int vector_field_j_offset);

  //
  // Methods which are leveraged both on internal sides in the kernel and by natural conditions
  //

  template <typename DiffusionHybridized>
  static void vectorFaceResidual(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void vectorFaceJacobian(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int lm_j_offset);

  template <typename DiffusionHybridized>
  static void scalarFaceResidual(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const MooseArray<Gradient> & vector_sol,
                                 const MooseArray<Number> & scalar_sol,
                                 const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void scalarFaceJacobian(DiffusionHybridized & obj,
                                 const unsigned int i_offset,
                                 const unsigned int vector_j_offset,
                                 const unsigned int scalar_j_offset,
                                 const unsigned int lm_j_offset);

  template <typename DiffusionHybridized>
  static void lmFaceResidual(DiffusionHybridized & obj,
                             const unsigned int i_offset,
                             const MooseArray<Gradient> & vector_sol,
                             const MooseArray<Number> & scalar_sol,
                             const MooseArray<Number> & lm_sol);

  template <typename DiffusionHybridized>
  static void lmFaceJacobian(DiffusionHybridized & obj,
                             const unsigned int i_offset,
                             const unsigned int vector_j_offset,
                             const unsigned int scalar_j_offset,
                             const unsigned int lm_j_offset);

private:
  /// optional source
  const Function & _source;

  friend class DiffusionHybridizedZeroFluxBC;
  friend class DiffusionHybridizedInterface;
};

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::vectorFaceResidual(DiffusionHybridized & obj,
                                              const unsigned int i_offset,
                                              const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    // Vector equation dependence on LM dofs
    for (const auto i : make_range(obj._vector_n_dofs))
      obj._PrimalVec(i_offset + i) -=
          obj._JxW_face[qp] * (obj._vector_phi_face[i][qp] * obj._normals[qp]) * lm_sol[qp];
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::vectorFaceJacobian(DiffusionHybridized & obj,
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

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::scalarFaceResidual(DiffusionHybridized & obj,
                                              const unsigned int i_offset,
                                              const MooseArray<Gradient> & vector_sol,
                                              const MooseArray<Number> & scalar_sol,
                                              const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      if (obj._neigh)
      {
        // vector
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._diff[qp] *
                                        obj._scalar_phi_face[i][qp] *
                                        (vector_sol[qp] * obj._normals[qp]);

        // scalar from stabilization term
        obj._PrimalVec(i_offset + i) += obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

        // lm from stabilization term
        obj._PrimalVec(i_offset + i) -= obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau *
                                        lm_sol[qp] * obj._normals[qp] * obj._normals[qp];
      }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::scalarFaceJacobian(DiffusionHybridized & obj,
                                              const unsigned int i_offset,
                                              const unsigned int vector_j_offset,
                                              const unsigned int scalar_j_offset,
                                              const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._scalar_n_dofs))
      if (obj._neigh)
      {
        for (const auto j : make_range(obj._vector_n_dofs))
          obj._PrimalMat(i_offset + i, vector_j_offset + j) -=
              obj._JxW_face[qp] * obj._diff[qp] * obj._scalar_phi_face[i][qp] *
              (obj._vector_phi_face[j][qp] * obj._normals[qp]);

        for (const auto j : make_range(obj._scalar_n_dofs))
          obj._PrimalMat(i_offset + i, scalar_j_offset + j) +=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];

        for (const auto j : make_range(obj._lm_n_dofs))
          // from stabilization term
          obj._PrimalLM(i_offset + i, lm_j_offset + j) -=
              obj._JxW_face[qp] * obj._scalar_phi_face[i][qp] * _tau * obj._lm_phi_face[j][qp] *
              obj._normals[qp] * obj._normals[qp];
      }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::lmFaceResidual(DiffusionHybridized & obj,
                                          const unsigned int i_offset,
                                          const MooseArray<Gradient> & vector_sol,
                                          const MooseArray<Number> & scalar_sol,
                                          const MooseArray<Number> & lm_sol)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      // vector
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._diff[qp] * obj._lm_phi_face[i][qp] *
                                  (vector_sol[qp] * obj._normals[qp]);

      // scalar from stabilization term
      obj._LMVec(i_offset + i) += obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau *
                                  scalar_sol[qp] * obj._normals[qp] * obj._normals[qp];

      // lm from stabilization term
      obj._LMVec(i_offset + i) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * lm_sol[qp] *
                                  obj._normals[qp] * obj._normals[qp];
    }
}

template <typename DiffusionHybridized>
void
DiffusionHybridizedKernel::lmFaceJacobian(DiffusionHybridized & obj,
                                          const unsigned int i_offset,
                                          const unsigned int vector_j_offset,
                                          const unsigned int scalar_j_offset,
                                          const unsigned int lm_j_offset)
{
  for (const auto qp : make_range(obj._qrule_face->n_points()))
    for (const auto i : make_range(obj._lm_n_dofs))
    {
      for (const auto j : make_range(obj._vector_n_dofs))
        obj._LMPrimal(i_offset + i, vector_j_offset + j) -=
            obj._JxW_face[qp] * obj._diff[qp] * obj._lm_phi_face[i][qp] *
            (obj._vector_phi_face[j][qp] * obj._normals[qp]);

      for (const auto j : make_range(obj._scalar_n_dofs))
        obj._LMPrimal(i_offset + i, scalar_j_offset + j) +=
            obj._JxW_face[qp] * obj._lm_phi_face[i][qp] * _tau * obj._scalar_phi_face[j][qp] *
            obj._normals[qp] * obj._normals[qp];

      for (const auto j : make_range(obj._lm_n_dofs))
        // from stabilization term
        obj._LMMat(i_offset + i, lm_j_offset + j) -= obj._JxW_face[qp] * obj._lm_phi_face[i][qp] *
                                                     _tau * obj._lm_phi_face[j][qp] *
                                                     obj._normals[qp] * obj._normals[qp];
    }
}

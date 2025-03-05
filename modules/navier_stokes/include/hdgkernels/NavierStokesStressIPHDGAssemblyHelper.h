//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DiffusionIPHDGAssemblyHelper.h"

/**
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the diffusion equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class NavierStokesStressIPHDGAssemblyHelper : public DiffusionIPHDGAssemblyHelper
{
public:
  static InputParameters validParams();

  NavierStokesStressIPHDGAssemblyHelper(const MooseObject * const moose_obj,
                                        MooseVariableDependencyInterface * const mvdi,
                                        const TransientInterface * const ti,
                                        const MooseMesh & mesh,
                                        SystemBase & sys,
                                        const Assembly & assembly,
                                        const THREAD_ID tid,
                                        const std::set<SubdomainID> & block_ids,
                                        const std::set<BoundaryID> & boundary_ids);

protected:
  /**
   * Computes a local residual vector for the weak form:
   * (Dq, grad(w)) - (f, w)
   * where D is the diffusivity, w are the test functions associated with the scalar field, and f is
   * a forcing function
   */
  virtual void scalarVolume() override;

  /**
   * Computes a local residual vector for the weak form:
   * -<Dq*n, w> + <\tau * (u - \hat{u}) * n * n, w>
   */
  virtual void scalarFace() override;

  virtual void scalarDirichlet(const Moose::Functor<Real> & dirichlet_value) override;

  /// The pressure variable on element interiors
  const MooseVariableFE<Real> & _pressure_var;
  /// The pressure variable on element faces
  const MooseVariableFE<Real> & _pressure_face_var;

  /// The pressure solution on element interiors
  const ADVariableValue & _pressure_sol;
  /// The pressure solution on element faces
  const ADVariableValue & _pressure_face_sol;

  /// The coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// The radial coordinate index for RZ coordinate systems
  const unsigned int _rz_radial_coord;

  /// The velocity component this object is adding a residual for
  unsigned int _component;
};

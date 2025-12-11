//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IPHDGAssemblyHelper.h"
#include "ADFunctorInterface.h"

class MooseMesh;

/**
 * Implements all the methods for assembling a hybridized interior penalty discontinuous Galerkin
 * (IPDG-H), which is a type of HDG method, discretization of the advection equation. These routines
 * may be called by both HDG kernels and integrated boundary conditions.
 */
class MassContinuityAssemblyHelper : public IPHDGAssemblyHelper, public ADFunctorInterface
{
public:
  static InputParameters validParams();

  MassContinuityAssemblyHelper(const MooseObject * const moose_obj,
                               MooseVariableDependencyInterface * const mvdi,
                               const TransientInterface * const ti,
                               const MooseMesh & mesh,
                               SystemBase & sys,
                               const Assembly & assembly,
                               const THREAD_ID tid,
                               const std::set<SubdomainID> & block_ids,
                               const std::set<BoundaryID> & boundary_ids);

  virtual void scalarVolume() override;
  virtual void scalarFace() override;
  virtual void lmFace() override;
  virtual void scalarDirichlet(const Moose::Functor<Real> &) override
  {
    mooseError("We do not assign Dirichlet values for pressure");
  }

  /// The coordinate system
  const Moose::CoordinateSystemType & _coord_sys;

  /// The radial coordinate index for RZ coordinate systems
  const unsigned int _rz_radial_coord;

  /// The velocities on the element interior
  std::vector<const ADVariableValue *> _interior_vels;

  /// The velocity gradients on the element interior
  std::vector<const ADVariableGradient *> _interior_vel_grads;

  /// The velocities on the element faces
  std::vector<const Moose::Functor<ADReal> *> _face_vels;
};

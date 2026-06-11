//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseTypes.h"

/**
 * Optional observer for native linear FV assembly coefficients.
 *
 * Linear FV kernels call this while adding the same coefficients to the tagged
 * matrix/RHS. Consumers can build a secondary equation object without re-running
 * kernel logic after PETSc assembly has flattened the FV contribution split.
 */
class LinearFVAssemblyConsumer
{
public:
  virtual ~LinearFVAssemblyConsumer() = default;

  virtual void addElementalMatrixContribution(dof_id_type dof, Real contribution) = 0;
  virtual void addElementalRightHandSideContribution(dof_id_type dof, Real contribution) = 0;

  virtual void addInternalFaceMatrixContribution(dof_id_type elem_dof,
                                                 dof_id_type neighbor_dof,
                                                 Real elem_matrix_contribution,
                                                 Real neighbor_matrix_contribution,
                                                 bool elem_has_blocks,
                                                 bool neighbor_has_blocks) = 0;
  virtual void addInternalFaceRightHandSideContribution(dof_id_type elem_dof,
                                                        dof_id_type neighbor_dof,
                                                        Real elem_rhs_contribution,
                                                        Real neighbor_rhs_contribution,
                                                        bool elem_has_blocks,
                                                        bool neighbor_has_blocks) = 0;

  virtual void
  addBoundaryMatrixContribution(dof_id_type face_id, dof_id_type dof, Real contribution) = 0;
  virtual void addBoundaryRightHandSideContribution(dof_id_type dof, Real contribution) = 0;
};

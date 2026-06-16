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
 * Optional observer for native linear FV boundary matrix coefficients.
 *
 * Linear FV kernels call this while adding the same boundary coefficients to the tagged matrix.
 * Consumers can preserve per-face boundary information that is not recoverable from the assembled
 * sparse matrix.
 */
class LinearFVAssemblyConsumer
{
public:
  virtual ~LinearFVAssemblyConsumer() = default;

  virtual void
  addBoundaryMatrixContribution(dof_id_type face_id, dof_id_type dof, Real contribution) = 0;
};

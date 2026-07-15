//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HybridizedDGAssemblyHelper.h"

/**
 * Base class that declares all the methods for assembling a hybridized interior penalty
 * discontinuous Galerkin (IPDG-H), which is a type of HDG method, discretization of an equation.
 * These routines may be called by both HDG kernels and integrated boundary conditions.
 */
class IPHDGAssemblyHelper : public HybridizedDGAssemblyHelper
{
public:
  static InputParameters validParams();

  IPHDGAssemblyHelper(const MooseObject * const moose_obj,
                      MooseVariableDependencyInterface * const mvdi,
                      const TransientInterface * const ti,
                      SystemBase & sys,
                      const Assembly & assembly,
                      const THREAD_ID tid,
                      const std::set<SubdomainID> & blocks_ids,
                      const std::set<BoundaryID> & boundary_ids);

  virtual ~IPHDGAssemblyHelper() = default;
};

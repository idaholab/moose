//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseMesh.h"
#include "TheWarehouse.h"

template <typename>
class MooseObjectTagWarehouse;
class IntegratedBCBase;

/**
 * Compose boundary restricted error message for the provided object, variables, and boundary_name
 * if the variables container is non-empty
 */
void boundaryIntegrityCheckError(const MooseObject & object,
                                 const std::set<MooseVariableFieldBase *> & variables,
                                 const BoundaryName & boundary_name);

class BoundaryElemIntegrityCheckThread
{
public:
  BoundaryElemIntegrityCheckThread(FEProblemBase & fe_problem, const TheWarehouse::Query & query);

  // Splitting Constructor
  BoundaryElemIntegrityCheckThread(BoundaryElemIntegrityCheckThread & x, Threads::split split);

  void operator()(const ConstBndElemRange & range);

  void join(const BoundaryElemIntegrityCheckThread & /*y*/);

protected:
  /// the finite element (or volume) problem
  FEProblemBase & _fe_problem;

  /// A warehouse query that we will use to obtain user objects for boundary variable dependency
  /// integrity checks
  const TheWarehouse::Query & _query;
};

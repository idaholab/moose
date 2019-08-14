//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeFullJacobianThread.h"

// Forward declarations
class FEProblemBase;

/**
 * Helper class for holding the preconditioning blocks to fill.
 */
class JacobianBlock
{
public:
  JacobianBlock(libMesh::System & precond_system,
                SparseMatrix<Number> & jacobian,
                unsigned int ivar,
                unsigned int jvar)
    : _precond_system(precond_system), _jacobian(jacobian), _ivar(ivar), _jvar(jvar)
  {
  }

  libMesh::System & _precond_system;
  SparseMatrix<Number> & _jacobian;
  unsigned int _ivar, _jvar;
};

/**
 * Specialization for filling multiple "small" preconditioning matrices simulatenously.
 */
class ComputeJacobianBlocksThread : public ComputeFullJacobianThread
{
public:
  ComputeJacobianBlocksThread(FEProblemBase & fe_problem,
                              std::vector<JacobianBlock *> & blocks,
                              const std::set<TagID> & tags);

  // Splitting Constructor
  ComputeJacobianBlocksThread(ComputeJacobianBlocksThread & x, Threads::split split);

  virtual ~ComputeJacobianBlocksThread();

  void join(const ComputeJacobianThread & /*y*/) {}

protected:
  virtual void postElement(const Elem * elem) override;

  virtual void postInternalSide(const Elem * elem, unsigned int side) override;

  std::vector<JacobianBlock *> _blocks;

private:
  std::vector<dof_id_type> _dof_indices;

  std::vector<dof_id_type> _dof_neighbor_indices;
};

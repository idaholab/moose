//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeResidualAndJacobianThread.h"
#include "NonlinearSystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "KernelBase.h"
#include "IntegratedBCBase.h"
#include "DGKernelBase.h"
#include "InterfaceKernelBase.h"
#include "Material.h"
#include "TimeKernel.h"
#include "SwapBackSentinel.h"
#include "FVTimeKernel.h"

#include "libmesh/threads.h"

ComputeResidualAndJacobianThread::ComputeResidualAndJacobianThread(
    FEProblemBase & fe_problem,
    const std::set<TagID> & vector_tags,
    const std::set<TagID> & matrix_tags)
  : NonlinearThread(fe_problem), _vector_tags(vector_tags), _matrix_tags(matrix_tags)
{
}

// Splitting Constructor
ComputeResidualAndJacobianThread::ComputeResidualAndJacobianThread(
    ComputeResidualAndJacobianThread & x, Threads::split split)
  : NonlinearThread(x, split), _vector_tags(x._vector_tags), _matrix_tags(x._matrix_tags)
{
}

ComputeResidualAndJacobianThread::~ComputeResidualAndJacobianThread() {}

void
ComputeResidualAndJacobianThread::compute(ResidualObject & ro)
{
  ro.computeResidualAndJacobian();
}

void
ComputeResidualAndJacobianThread::accumulateLower()
{
  _fe_problem.addResidualLower(_tid);
  _fe_problem.addJacobianLowerD(_tid);
}

void
ComputeResidualAndJacobianThread::accumulateNeighborLower()
{
  _fe_problem.addResidualNeighbor(_tid);
  _fe_problem.addResidualLower(_tid);
  _fe_problem.addJacobianNeighborLowerD(_tid);
}

void
ComputeResidualAndJacobianThread::accumulateNeighbor()
{
  _fe_problem.addResidualNeighbor(_tid);
  _fe_problem.addJacobianNeighbor(_tid);
}

void
ComputeResidualAndJacobianThread::accumulate()
{
  _fe_problem.cacheResidual(_tid);
  _fe_problem.cacheJacobian(_tid);
  _num_cached++;

  if (_num_cached % 20 == 0)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    _fe_problem.addCachedResidual(_tid);
    _fe_problem.addCachedJacobian(_tid);
  }
}

void
ComputeResidualAndJacobianThread::join(const ComputeResidualAndJacobianThread & /*y*/)
{
}

void
ComputeResidualAndJacobianThread::determineObjectWarehouses()
{
  if (_vector_tags.size() &&
      _vector_tags.size() != _fe_problem.numVectorTags(Moose::VECTOR_TAG_RESIDUAL))
    mooseError("Can only currently compute the residual and Jacobian together if we are computing "
               "the full suite of residual tags");

  if (_matrix_tags.size() && _matrix_tags.size() != _fe_problem.numMatrixTags())
    mooseError("Can only currently compute the residual and Jacobian together if we are computing "
               "the full suite of Jacobian tags");

  _tag_kernels = &_kernels;
  _dg_warehouse = &_dg_kernels;
  _ibc_warehouse = &_integrated_bcs;
  _ik_warehouse = &_interface_kernels;

  if (_fe_problem.haveFV())
  {
    _fv_kernels.clear();
    _fe_problem.theWarehouse()
        .query()
        .template condition<AttribSysNum>(_nl.number())
        .template condition<AttribSystem>("FVElementalKernel")
        .template condition<AttribSubdomains>(_subdomain)
        .template condition<AttribThread>(_tid)
        .queryInto(_fv_kernels);
  }
}

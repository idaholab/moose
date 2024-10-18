//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaxQpsThread.h"
#include "FEProblem.h"
#include "Assembly.h"

#include "libmesh/fe.h"
#include "libmesh/threads.h"
#include LIBMESH_INCLUDE_UNORDERED_SET
LIBMESH_DEFINE_HASH_POINTERS
#include "libmesh/quadrature.h"

MaxQpsThread::MaxQpsThread(FEProblemBase & fe_problem) : _fe_problem(fe_problem), _max(0) {}

// Splitting Constructor
MaxQpsThread::MaxQpsThread(MaxQpsThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem), _max(x._max)
{
}

void
MaxQpsThread::operator()(const libMesh::ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // Not actually using any pre-existing data so it shouldn't matter which assembly we use
  auto & assem = _fe_problem.assembly(_tid, 0);

  // For short circuiting reinit.  With potential block-specific qrules we
  // need to track "seen" element types by their subdomains as well.
  std::set<std::pair<ElemType, SubdomainID>> seen_it;

  for (const auto & elem : range)
  {
    // Only reinit if the element type has not previously been seen
    if (!seen_it.insert(std::make_pair(elem->type(), elem->subdomain_id())).second)
      continue;

    // This ensures we can access the correct qrules if any block-specific
    // qrules have been created.
    assem.setCurrentSubdomainID(elem->subdomain_id());
    assem.setVolumeQRule(elem);

    auto & qrule = assem.writeableQRule();
    qrule->init(elem->type(), elem->p_level());
    if (qrule->n_points() > _max)
      _max = qrule->n_points();

    if (elem->dim())
    {
      // We are assuming here that side 0 ends up with at least as many quadrature points as any
      // other side
      assem.setFaceQRule(elem, /*side=*/0);
      auto & qrule_face = assem.writeableQRuleFace();
      qrule_face->init(elem->side_type(0), elem->p_level());
      if (qrule_face->n_points() > _max)
        _max = qrule->n_points();
    }

    // In initial conditions nodes are enumerated as pretend quadrature points
    // using the _qp index to access coupled variables. In order to be able to
    // use _zero (resized according to _max_qps) with _qp, we need to count nodes.
    if (elem->n_nodes() > _max)
      _max = elem->n_nodes();
  }

  // Clear the cached quadrature rules because we may add FE objects in between now and simulation
  // start and we only ensure we set all the FE quadrature rules if a quadrature rule is different
  // from the cached quadrature rule
  assem.clearCachedQRules();
}

void
MaxQpsThread::join(const MaxQpsThread & y)
{
  if (y._max > _max)
    _max = y._max;
}

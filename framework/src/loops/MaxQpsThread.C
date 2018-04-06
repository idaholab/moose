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

#include "libmesh/fe_base.h"
#include "libmesh/threads.h"
#include LIBMESH_INCLUDE_UNORDERED_SET
LIBMESH_DEFINE_HASH_POINTERS
#include "libmesh/quadrature.h"

MaxQpsThread::MaxQpsThread(FEProblemBase & fe_problem,
                           QuadratureType qtype,
                           Order order,
                           Order face_order)
  : _fe_problem(fe_problem),
    _qtype(qtype),
    _order(order),
    _face_order(face_order),
    _max(0),
    _max_shape_funcs(0)
{
}

// Splitting Constructor
MaxQpsThread::MaxQpsThread(MaxQpsThread & x, Threads::split /*split*/)
  : _fe_problem(x._fe_problem),
    _qtype(x._qtype),
    _order(x._order),
    _face_order(x._face_order),
    _max(x._max),
    _max_shape_funcs(x._max_shape_funcs)
{
}

void
MaxQpsThread::operator()(const ConstElemRange & range)
{
  ParallelUniqueId puid;
  _tid = puid.id;

  // For short circuiting reinit
  std::set<ElemType> seen_it;
  for (const auto & elem : range)
  {
    // Only reinit if the element type has not previously been seen
    if (seen_it.insert(elem->type()).second)
    {
      FEType fe_type(FIRST, LAGRANGE);
      unsigned int dim = elem->dim();
      unsigned int side = 0; // we assume that any element will have at least one side ;)

      // We cannot mess with the FE objects in Assembly, because we might need to request second
      // derivatives
      // later on. If we used them, we'd call reinit on them, thus making the call to request second
      // derivatives harmful (i.e. leading to segfaults/asserts). Thus, we have to use a locally
      // allocated object here.
      std::unique_ptr<FEBase> fe(FEBase::build(dim, fe_type));

      // figure out the number of qps for volume
      std::unique_ptr<QBase> qrule(QBase::build(_qtype, dim, _order));
      fe->attach_quadrature_rule(qrule.get());
      fe->reinit(elem);
      if (qrule->n_points() > _max)
        _max = qrule->n_points();

      unsigned int n_shape_funcs = fe->n_shape_functions();
      if (n_shape_funcs > _max_shape_funcs)
        _max_shape_funcs = n_shape_funcs;

      // figure out the number of qps for the face
      // NOTE: user might specify higher order rule for faces, thus possibly ending up with more qps
      // than in the volume
      std::unique_ptr<QBase> qrule_face(QBase::build(_qtype, dim - 1, _face_order));
      fe->attach_quadrature_rule(qrule_face.get());
      fe->reinit(elem, side);
      if (qrule_face->n_points() > _max)
        _max = qrule_face->n_points();

      // In initial conditions nodes are enumerated as pretend quadrature points
      // using the _qp index to access coupled variables. In order to be able to
      // use _zero (resized according to _max_qps) with _qp, we need to count nodes.
      if (elem->n_nodes() > _max)
        _max = elem->n_nodes();
    }
  }
}

void
MaxQpsThread::join(const MaxQpsThread & y)
{
  if (y._max > _max)
    _max = y._max;

  if (y._max_shape_funcs > _max_shape_funcs)
    _max_shape_funcs = y._max_shape_funcs;
}

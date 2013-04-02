/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "NodalNormalsEvaluator.h"

Threads::spin_mutex nodal_normals_evaluator_mutex;

template<>
InputParameters validParams<NodalNormalsEvaluator>()
{
  InputParameters params = validParams<NodalUserObject>();

  return params;
}

NodalNormalsEvaluator::NodalNormalsEvaluator(const std::string & name, InputParameters parameters) :
    NodalUserObject(name, parameters),
    _nx(_fe_problem.getAuxiliarySystem().getVector("nx")),
    _ny(_fe_problem.getAuxiliarySystem().getVector("ny")),
    _nz(_fe_problem.getAuxiliarySystem().getVector("nz"))
{
}

NodalNormalsEvaluator::~NodalNormalsEvaluator()
{
}

void
NodalNormalsEvaluator::execute()
{
  Threads::spin_mutex::scoped_lock lock(nodal_normals_evaluator_mutex);

  dof_id_type dof = _current_node->id();
  Real n = std::sqrt((_nx(dof) * _nx(dof)) + (_ny(dof) * _ny(dof)) + (_nz(dof) * _nz(dof)));
  if (std::abs(n) >= 1e-13)
  {
    // divide by n only if it is not close to zero to avoid NaNs
    _nx.set(dof, _nx(dof) / n);
    _ny.set(dof, _ny(dof) / n);
    _nz.set(dof, _nz(dof) / n);
  }
}

void
NodalNormalsEvaluator::initialize()
{
}

void
NodalNormalsEvaluator::destroy()
{
}

void
NodalNormalsEvaluator::finalize()
{
  _nx.close();
  _ny.close();
  _nz.close();
}

void
NodalNormalsEvaluator::threadJoin(const UserObject & /*uo*/)
{
}

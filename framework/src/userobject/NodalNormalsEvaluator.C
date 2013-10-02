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
  params.set<bool>("_dual_restrictable") = true;
  return params;
}

NodalNormalsEvaluator::NodalNormalsEvaluator(const std::string & name, InputParameters parameters) :
    NodalUserObject(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem())
{
}

NodalNormalsEvaluator::~NodalNormalsEvaluator()
{
}

void
NodalNormalsEvaluator::execute()
{
  if (_current_node->processor_id() == libMesh::processor_id())
  {
    if (_current_node->n_dofs(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_x").number()) > 0)
    {
      Threads::spin_mutex::scoped_lock lock(nodal_normals_evaluator_mutex);

      dof_id_type dof_x = _current_node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_x").number(), 0);
      dof_id_type dof_y = _current_node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_y").number(), 0);
      dof_id_type dof_z = _current_node->dof_number(_aux.number(), _fe_problem.getVariable(_tid, "nodal_normal_z").number(), 0);

      NumericVector<Number> & sln = _aux.solution();
      Real nx = sln(dof_x);
      Real ny = sln(dof_y);
      Real nz = sln(dof_z);

      Real n = std::sqrt((nx * nx) + (ny * ny) + (nz * nz));
      if (std::abs(n) >= 1e-13)
      {
        // divide by n only if it is not close to zero to avoid NaNs
        sln.set(dof_x, nx / n);
        sln.set(dof_y, ny / n);
        sln.set(dof_z, nz / n);
      }
    }
  }
}

void
NodalNormalsEvaluator::initialize()
{
}

void
NodalNormalsEvaluator::finalize()
{
  _aux.solution().close();
}

void
NodalNormalsEvaluator::threadJoin(const UserObject & /*uo*/)
{
}

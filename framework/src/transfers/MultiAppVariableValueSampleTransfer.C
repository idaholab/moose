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

#include "MultiAppVariableValueSampleTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppVariableValueSampleTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppVariableValueSampleTransfer::MultiAppVariableValueSampleTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
}

void
MultiAppVariableValueSampleTransfer::execute()
{
  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
      SystemBase & from_system_base = from_var.sys();
      SubProblem & from_sub_problem = from_system_base.subproblem();

      System * from_sys = find_sys(from_problem.es(), _from_var_name);

      MooseMesh & from_mesh = from_problem.mesh();

      AutoPtr<PointLocatorBase> pl = from_mesh.getMesh().sub_point_locator();

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        Real value = -std::numeric_limits<Real>::max();

        { // Get the value of the variable at the point where this multiapp is in the master domain

          Point multi_app_position = _multi_app->position(i);

          std::vector<Point> point_vec(1, multi_app_position);

          // First find the element the hit lands in
          const Elem * elem = (*pl)(multi_app_position);

          if(elem && elem->processor_id() == libMesh::processor_id())
          {
            from_sub_problem.reinitElemPhys(elem, point_vec, 0);

            mooseAssert(_u.size() == 1, "No values in u!");
            value = from_var.sln()[0];
          }

          libMesh::Parallel::max(value);
        }

        if(_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = *to_sys->solution;

          MooseMesh & mesh = _multi_app->appProblem(i)->mesh();

          MeshBase::const_node_iterator node_it = mesh.local_nodes_begin();
          MeshBase::const_node_iterator node_end = mesh.local_nodes_end();

          for(; node_it != node_end; ++node_it)
          {
            Node * node = *node_it;

            if(node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
            {
              // The zero only works for LAGRANGE!
              unsigned int dof = node->dof_number(sys_num, var_num, 0);

              solution.set(dof, value);
            }
          }
          solution.close();
          _multi_app->appProblem(i)->es().update();

          // Swap back
          Moose::swapLibMeshComm(swapped);
        }
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      mooseError("Doesn't make sense to transfer a sampled variable's value from a MultiApp!!");
      break;
    }
  }
}

/**
 * Small helper function for finding the system containing the variable.
 *
 * Note that this implies that variable names are unique across all systems!
 */
System *
MultiAppVariableValueSampleTransfer::find_sys(EquationSystems & es, std::string & var_name)
{
  System * sys = NULL;

  // Find the system this variable is from
  for(unsigned int i=0; i<es.n_systems(); i++)
  {
    if(es.get_system(i).has_variable(var_name))
    {
      sys = &es.get_system(i);
      break;
    }
  }

  mooseAssert(sys, "Unable to find variable " + var_name);

  return sys;
}

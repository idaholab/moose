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

#include "MultiAppMeshFunctionTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"

template<>
InputParameters validParams<MultiAppMeshFunctionTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  return params;
}

MultiAppMeshFunctionTransfer::MultiAppMeshFunctionTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
}

void
MultiAppMeshFunctionTransfer::execute()
{
  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
      SystemBase & from_system_base = from_var.sys();

      System & from_sys = from_system_base.system();

      // Only works with a serialized mesh to transfer from!
      mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppMeshFunctionTransfer only works with SerialMesh!");

      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      EquationSystems & from_es = from_sys.get_equation_systems();

      //Create a serialized version of the solution vector
      NumericVector<Number> * serialized_solution = NumericVector<Number>::build().release();
      serialized_solution->init(from_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in parallel
      from_sys.solution->localize(*serialized_solution);

      MeshFunction from_func(from_es, *serialized_solution, from_sys.get_dof_map(), from_var_num);
      from_func.init();

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        std::cout<<"MeshFunction Transfer To: "<<_multi_app->name()<<i<<std::endl;
        if(_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = *to_sys->solution;

          MooseMesh & mesh = _multi_app->appProblem(i)->mesh();

          MeshBase::const_node_iterator node_it = mesh.localNodesBegin();
          MeshBase::const_node_iterator node_end = mesh.localNodesEnd();

          for(; node_it != node_end; ++node_it)
          {
            Node * node = *node_it;

            if(node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
            {
              // The zero only works for LAGRANGE!
              unsigned int dof = node->dof_number(sys_num, var_num, 0);

              // Swap back
              Moose::swapLibMeshComm(swapped);
              Real from_value = from_func(*node+_multi_app->position(i));
              // Swap again
              swapped = Moose::swapLibMeshComm(_multi_app->comm());

              solution.set(dof, from_value);
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
      mooseError("Not Implemented!");
      break;
    }
  }
}

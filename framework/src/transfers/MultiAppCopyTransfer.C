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

// MOOSE includes
#include "MultiAppCopyTransfer.h"
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"
#include "MultiApp.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/id_types.h"

template<>
InputParameters validParams<MultiAppCopyTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");

  return params;
}

MultiAppCopyTransfer::MultiAppCopyTransfer(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable"))
{
  // This transfer does not work with ParallelMesh
  _fe_problem.mesh().errorIfParallelDistribution("MultiAppCopyTransfer");
}

void
MultiAppCopyTransfer::initialSetup()
{
  variableIntegrityCheck(_to_var_name);
}

void
MultiAppCopyTransfer::execute()
{
  _console << "Beginning CopyTransfer " << name() << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = _multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);

      MeshBase * from_mesh = NULL;

      from_mesh = &from_problem.mesh().getMesh();

      SystemBase & from_system_base = from_var.sys();

      System & from_sys = from_system_base.system();
      unsigned int from_sys_num = from_sys.number();

      // Only works with a serialized mesh to transfer from!
      mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppCopyTransfer only works with ReplicatedMesh!");

      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      //Create a serialized version of the solution vector
//      NumericVector<Number> * serialized_solution = NumericVector<Number>::build(from_sys.comm()).release();
//      serialized_solution->init(from_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in parallel
//      from_sys.solution->localize(*serialized_solution);

      for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i).es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase * mesh = NULL;

          mesh = &_multi_app->appProblem(i).mesh().getMesh();

          bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

          if (is_nodal)
          {
            MeshBase::const_node_iterator node_it = mesh->local_nodes_begin();
            MeshBase::const_node_iterator node_end = mesh->local_nodes_end();

            for (; node_it != node_end; ++node_it)
            {
              Node * node = *node_it;
              unsigned int node_id = node->id();

              if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                // Swap back
                Moose::swapLibMeshComm(swapped);

                Node * from_node = from_mesh->node_ptr(node_id);

                // Assuming LAGRANGE!
                dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);
                //Real from_value = (*serialized_solution)(from_dof);
                Real from_value = (*from_sys.solution)(from_dof);

                // Swap again
                swapped = Moose::swapLibMeshComm(_multi_app->comm());

                solution.set(dof, from_value);
              }
            }
          }
          else // Elemental
          {
            mooseError("MultiAppCopyTransfer can only be used on nodal variables");
          }

          solution.close();
          to_sys->update();

          // Swap back
          Moose::swapLibMeshComm(swapped);
        }
      }

//      delete serialized_solution;

      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblem & to_problem = _multi_app->problem();
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      NumericVector<Real> & to_solution = *to_sys.solution;

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(), "MultiAppCopyTransfer only works with ReplicatedMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      MeshBase * to_mesh = NULL;

      to_mesh = &to_problem.mesh().getMesh();

      bool is_nodal = to_sys.variable_type(to_var_num) == FEType();

      for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if (!_multi_app->hasLocalApp(i))
          continue;

        MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

        FEProblem & from_problem = _multi_app->appProblem(i);
        MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();

        unsigned int from_sys_num = from_sys.number();

        unsigned int from_var_num = from_sys.variable_number(from_var.name());

        // Only works with a serialized mesh to transfer from!
        mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppCopyTransfer only works with ReplicatedMesh!");

        //Create a serialized version of the solution vector
//        NumericVector<Number> * serialized_solution = NumericVector<Number>::build(from_sys.comm()).release();
//        serialized_solution->init(from_sys.n_dofs(), false, SERIAL);

        // Need to pull down a full copy of this vector on every processor so we can get values in parallel
//        from_sys.solution->localize(*serialized_solution);


        MeshBase * from_mesh = &from_problem.mesh().getMesh();

        Moose::swapLibMeshComm(swapped);

        if (is_nodal)
        {
          MeshBase::const_node_iterator to_node_it = to_mesh->local_nodes_begin();
          MeshBase::const_node_iterator to_node_end = to_mesh->local_nodes_end();

          for (; to_node_it != to_node_end; ++to_node_it)
          {
            Node * to_node = *to_node_it;
            unsigned int to_node_id = to_node->id();

            // The zero only works for LAGRANGE!
            dof_id_type to_dof = to_node->dof_number(to_sys_num, to_var_num, 0);

            MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

            Node * from_node = from_mesh->node_ptr(to_node_id);

            // Assuming LAGRANGE!
            dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);
            //Real from_value = (*serialized_solution)(from_dof);
            Real from_value = (*from_sys.solution)(from_dof);

            // Swap back
            Moose::swapLibMeshComm(swapped);

            to_solution.set(to_dof, from_value);
          }
        }
        else // Elemental
        {
          mooseError("MultiAppCopyTransfer can only be used on nodal variables");
        }

//        delete serialized_solution;
      }

      to_solution.close();
      to_sys.update();

      break;
    }
  }

  _console << "Finished CopyTransfer " << name() << std::endl;
}

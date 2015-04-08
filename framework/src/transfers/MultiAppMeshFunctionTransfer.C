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
#include "DisplacedProblem.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<MultiAppMeshFunctionTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("displaced_source_mesh", false, "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh", false, "Whether or not to use the displaced mesh for the target mesh.");
  params.addParam<bool>("error_on_miss", false, "Whether or not to error in the case that a target point is not found in the source domain.");
  return params;
}

MultiAppMeshFunctionTransfer::MultiAppMeshFunctionTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _displaced_source_mesh(getParam<bool>("displaced_source_mesh")),
    _displaced_target_mesh(getParam<bool>("displaced_target_mesh")),
    _error_on_miss(getParam<bool>("error_on_miss"))
{
  // This transfer does not work with ParallelMesh
  _fe_problem.mesh().errorIfParallelDistribution("MultiAppMeshFunctionTransfer");
}

void
MultiAppMeshFunctionTransfer::initialSetup()
{
  if (_direction == TO_MULTIAPP)
    variableIntegrityCheck(_to_var_name);
  else
    variableIntegrityCheck(_from_var_name);
}

void
MultiAppMeshFunctionTransfer::execute()
{
  Moose::out << "Beginning MeshFunctionTransfer " << _name << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      // TODO: This doesn't work with the master app being displaced see #3424
      if (_displaced_source_mesh)
        mooseError("displaced_source_mesh is not yet implemented for transferring 'to_multiapp'");

      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);

      SystemBase & from_system_base = from_var.sys();

      System & from_sys = from_system_base.system();

      // Only works with a serialized mesh to transfer from!
      mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppMeshFunctionTransfer only works with SerialMesh!");

      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      EquationSystems * tmp_es = NULL;

      if (_displaced_source_mesh && from_problem.getDisplacedProblem())
        tmp_es = &from_problem.getDisplacedProblem()->es();
      else
        tmp_es = &from_problem.es();

      EquationSystems & from_es = *tmp_es;

      //Create a serialized version of the solution vector
      NumericVector<Number> * serialized_solution = NumericVector<Number>::build(from_sys.comm()).release();
      serialized_solution->init(from_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in parallel
      from_sys.solution->localize(*serialized_solution);

      MeshFunction from_func(from_es, *serialized_solution, from_sys.get_dof_map(), from_var_num);
      from_func.init(Trees::ELEMENTS);
      from_func.enable_out_of_mesh_mode(OutOfMeshValue);

      for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);
          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase * tmp_mesh = NULL;

          if (_displaced_target_mesh && _multi_app->appProblem(i)->getDisplacedProblem())
            tmp_mesh = &_multi_app->appProblem(i)->getDisplacedProblem()->mesh().getMesh();
          else
            tmp_mesh = &_multi_app->appProblem(i)->mesh().getMesh();

          MeshBase & mesh = *tmp_mesh;

          bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

          if (is_nodal)
          {
            MeshBase::const_node_iterator node_it = mesh.local_nodes_begin();
            MeshBase::const_node_iterator node_end = mesh.local_nodes_end();

            for (; node_it != node_end; ++node_it)
            {
              Node * node = *node_it;

              if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                // Swap back
                Moose::swapLibMeshComm(swapped);
                Real from_value = from_func(*node+_multi_app->position(i));
                // Swap again
                swapped = Moose::swapLibMeshComm(_multi_app->comm());

                if (from_value != OutOfMeshValue)
                  solution.set(dof, from_value);
                else if (_error_on_miss)
                  mooseError("Point not found! " << *node+_multi_app->position(i) << std::endl);
              }
            }
          }
          else // Elemental
          {
            MeshBase::const_element_iterator elem_it = mesh.local_elements_begin();
            MeshBase::const_element_iterator elem_end = mesh.local_elements_end();

            for (; elem_it != elem_end; ++elem_it)
            {
              Elem * elem = *elem_it;

              Point centroid = elem->centroid();

              if (elem->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this elem
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = elem->dof_number(sys_num, var_num, 0);

                // Swap back
                Moose::swapLibMeshComm(swapped);
                Real from_value = from_func(centroid+_multi_app->position(i));
                // Swap again
                swapped = Moose::swapLibMeshComm(_multi_app->comm());

                if (from_value != OutOfMeshValue)
                  solution.set(dof, from_value);
                else if (_error_on_miss)
                  mooseError("Point not found! " << centroid+_multi_app->position(i) << std::endl);
              }
            }
          }

          solution.close();
          to_sys->update();

          // Swap back
          Moose::swapLibMeshComm(swapped);
        }
      }

      delete serialized_solution;

      break;
    }
    case FROM_MULTIAPP:
    {
      // TODO: This doesn't work with the master app being displaced see #3424
      if (_displaced_target_mesh)
        mooseError("displaced_target_mesh is not yet implemented for transferring 'from_multiapp'");

      FEProblem & to_problem = *_multi_app->problem();
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(), "MultiAppMeshFunctionTransfer only works with SerialMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      NumericVector<Number> * to_solution = to_sys.solution.get();

      MeshBase * tmp_to_mesh = NULL;

      if (_displaced_target_mesh && to_problem.getDisplacedProblem())
        tmp_to_mesh = &to_problem.getDisplacedProblem()->mesh().getMesh();
      else
        tmp_to_mesh = &to_problem.mesh().getMesh();

      MeshBase & to_mesh = *tmp_to_mesh;

      bool is_nodal = to_sys.variable_type(to_var_num).family == LAGRANGE;

      for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if (!_multi_app->hasLocalApp(i))
          continue;

        MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
        FEProblem & from_problem = *_multi_app->appProblem(i);
        MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();

        // Only works with a serialized mesh to transfer from!
        mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppMeshFunctionTransfer only works with SerialMesh!");

        unsigned int from_var_num = from_sys.variable_number(from_var.name());

        EquationSystems * tmp_es = NULL;

        if (_displaced_source_mesh && from_problem.getDisplacedProblem())
          tmp_es = &from_problem.getDisplacedProblem()->es();
        else
          tmp_es = &from_problem.es();

        EquationSystems & from_es = *tmp_es;

        //Create a serialized version of the solution vector
        NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build(from_sys.comm()).release();
        serialized_from_solution->init(from_sys.n_dofs(), false, SERIAL);

        // Need to pull down a full copy of this vector on every processor so we can get values in parallel
        from_sys.solution->localize(*serialized_from_solution);

        MeshBase * from_mesh = NULL;

        if (_displaced_source_mesh && from_problem.getDisplacedProblem())
          from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
        else
          from_mesh = &from_problem.mesh().getMesh();

        MeshTools::BoundingBox app_box = MeshTools::processor_bounding_box(*from_mesh, from_mesh->processor_id());
        Point app_position = _multi_app->position(i);

        MeshFunction from_func(from_es, *serialized_from_solution, from_sys.get_dof_map(), from_var_num);
        from_func.init(Trees::ELEMENTS);
        from_func.enable_out_of_mesh_mode(OutOfMeshValue);
        Moose::swapLibMeshComm(swapped);

        if (is_nodal)
        {
          MeshBase::const_node_iterator node_it = to_mesh.nodes_begin();
          MeshBase::const_node_iterator node_end = to_mesh.nodes_end();

          for (; node_it != node_end; ++node_it)
          {
            Node * node = *node_it;

            if (node->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this node
            {
              // See if this node falls in this bounding box
              if (app_box.contains_point(*node-app_position))
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);

                MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
                Real from_value = from_func(*node-app_position);
                Moose::swapLibMeshComm(swapped);

                if (from_value != OutOfMeshValue)
                  to_solution->set(dof, from_value);
                else if (_error_on_miss)
                  mooseError("Point not found! " << *node-app_position <<std::endl);
              }
            }
          }
        }
        else // Elemental
        {
          MeshBase::const_element_iterator elem_it = to_mesh.elements_begin();
          MeshBase::const_element_iterator elem_end = to_mesh.elements_end();

          for (; elem_it != elem_end; ++elem_it)
          {
            Elem * elem = *elem_it;

            if (elem->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this elem
            {
              Point centroid = elem->centroid();

              // See if this elem falls in this bounding box
              if (app_box.contains_point(centroid-app_position))
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, 0);

                MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
                Real from_value = from_func(centroid-app_position);
                Moose::swapLibMeshComm(swapped);

                if (from_value != OutOfMeshValue)
                  to_solution->set(dof, from_value);
                else if (_error_on_miss)
                  mooseError("Point not found! " << centroid-app_position << std::endl);
              }
            }
          }
        }
        delete serialized_from_solution;
      }

      to_solution->close();
      to_sys.update();

      break;
    }
  }

  _console << "Finished MeshFunctionTransfer " << _name << std::endl;
}

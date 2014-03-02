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

#define NOTFOUND -999999

#include "MultiAppMeshFunctionTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"

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
  params.addParam<bool>("error_on_miss", false, "Whether or not to error in the case that a target point is not found in the source domain.");
  return params;
}

MultiAppMeshFunctionTransfer::MultiAppMeshFunctionTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _error_on_miss(getParam<bool>("error_on_miss"))
{
  // This transfer does not work with ParallelMesh
  _fe_problem.mesh().errorIfParallelDistribution("MultiAppMeshFunctionTransfer");
}

void
MultiAppMeshFunctionTransfer::execute()
{
  Moose::out << "Beginning MeshFunctionTransfer " << _name << std::endl;

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
      from_func.init(Trees::ELEMENTS);
      from_func.enable_out_of_mesh_mode(NOTFOUND);

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          if (!to_sys)
            mooseError("Cannot find variable "<<_to_var_name<<" for "<<_name<<" Transfer");

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);
          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase & mesh = _multi_app->appProblem(i)->mesh().getMesh();
          bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

          if (is_nodal)
          {
            MeshBase::const_node_iterator node_it = mesh.local_nodes_begin();
            MeshBase::const_node_iterator node_end = mesh.local_nodes_end();

            for(; node_it != node_end; ++node_it)
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

                if (from_value != NOTFOUND)
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

            for(; elem_it != elem_end; ++elem_it)
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

                if (from_value != NOTFOUND)
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
      FEProblem & to_problem = *_multi_app->problem();
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(), "MultiAppMeshFunctionTransfer only works with SerialMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      EquationSystems & to_es = to_sys.get_equation_systems();

      NumericVector<Number> * to_solution = to_sys.solution.get();

      MeshBase & to_mesh = to_es.get_mesh();

      bool is_nodal = to_sys.variable_type(to_var_num).family == LAGRANGE;

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
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

        EquationSystems & from_es = from_sys.get_equation_systems();

        //Create a serialized version of the solution vector
        NumericVector<Number> * serialized_from_solution = NumericVector<Number>::build().release();
        serialized_from_solution->init(from_sys.n_dofs(), false, SERIAL);

        // Need to pull down a full copy of this vector on every processor so we can get values in parallel
        from_sys.solution->localize(*serialized_from_solution);

        MeshBase & from_mesh = from_es.get_mesh();
        MeshTools::BoundingBox app_box = MeshTools::processor_bounding_box(from_mesh, libMesh::processor_id());
        Point app_position = _multi_app->position(i);

        MeshFunction from_func(from_es, *serialized_from_solution, from_sys.get_dof_map(), from_var_num);
        from_func.init(Trees::ELEMENTS);
        from_func.enable_out_of_mesh_mode(NOTFOUND);
        Moose::swapLibMeshComm(swapped);

        if (is_nodal)
        {
          MeshBase::const_node_iterator node_it = to_mesh.nodes_begin();
          MeshBase::const_node_iterator node_end = to_mesh.nodes_end();

          for(; node_it != node_end; ++node_it)
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

                if (from_value != NOTFOUND)
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

          for(; elem_it != elem_end; ++elem_it)
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

                if (from_value != NOTFOUND)
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

  Moose::out << "Finished MeshFunctionTransfer " << _name << std::endl;
}


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

#include "MultiAppNearestNodeTransfer.h"

// Moose
#include "MooseTypes.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

// libMesh
#include "libmesh/system.h"
#include "libmesh/mesh_tools.h"

template<>
InputParameters validParams<MultiAppNearestNodeTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>("variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<VariableName>("source_variable", "The variable to transfer from.");
  params.addParam<bool>("displaced_source_mesh", false, "Whether or not to use the displaced mesh for the source mesh.");
  params.addParam<bool>("displaced_target_mesh", false, "Whether or not to use the displaced mesh for the target mesh.");
  return params;
}

MultiAppNearestNodeTransfer::MultiAppNearestNodeTransfer(const std::string & name, InputParameters parameters) :
    MultiAppTransfer(name, parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _from_var_name(getParam<VariableName>("source_variable")),
    _displaced_source_mesh(getParam<bool>("displaced_source_mesh")),
    _displaced_target_mesh(getParam<bool>("displaced_target_mesh"))
{
}

void
MultiAppNearestNodeTransfer::execute()
{
  std::cout<<"Beginning NearestNodeTransfer!"<<std::endl;

  switch(_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblem & from_problem = *_multi_app->problem();
      MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);

      MeshBase * from_mesh = NULL;

      if(_displaced_source_mesh && from_problem.getDisplacedProblem())
        from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
      else
        from_mesh = &from_problem.mesh().getMesh();

      SystemBase & from_system_base = from_var.sys();

      System & from_sys = from_system_base.system();
      unsigned int from_sys_num = from_sys.number();

      // Only works with a serialized mesh to transfer from!
      mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppNearestNodeTransfer only works with SerialMesh!");

      unsigned int from_var_num = from_sys.variable_number(from_var.name());

      EquationSystems & from_es = from_sys.get_equation_systems();

      //Create a serialized version of the solution vector
      NumericVector<Number> * serialized_solution = NumericVector<Number>::build().release();
      serialized_solution->init(from_sys.n_dofs(), false, SERIAL);

      // Need to pull down a full copy of this vector on every processor so we can get values in parallel
      from_sys.solution->localize(*serialized_solution);

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if(_multi_app->hasLocalApp(i))
        {
          MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblem(i)->es(), _to_var_name);

          if(!to_sys)
            mooseError("Cannot find variable "<<_to_var_name<<" for "<<_name<<" Transfer");

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);
          NumericVector<Real> & solution = *to_sys->solution;

          MeshBase * mesh = NULL;

          if(_displaced_target_mesh && _multi_app->appProblem(i)->getDisplacedProblem())
            mesh = &_multi_app->appProblem(i)->getDisplacedProblem()->mesh().getMesh();
          else
            mesh = &_multi_app->appProblem(i)->mesh().getMesh();

          bool is_nodal = to_sys->variable_type(var_num) == FEType();

          if(is_nodal)
          {
            MeshBase::const_node_iterator node_it = mesh->local_nodes_begin();
            MeshBase::const_node_iterator node_end = mesh->local_nodes_end();

            for(; node_it != node_end; ++node_it)
            {
              Node * node = *node_it;

              Point actual_position = *node+_multi_app->position(i);

              if(node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                // The zero only works for LAGRANGE!
                unsigned int dof = node->dof_number(sys_num, var_num, 0);

                // Swap back
                Moose::swapLibMeshComm(swapped);

                Real distance = 0; // Just to satisfy the last argument

                Node * nearest_node = getNearestNode(*from_mesh, actual_position, distance);

                // Assuming LAGRANGE!
                unsigned int from_dof = nearest_node->dof_number(from_sys_num, from_var_num, 0);
                Real from_value = (*serialized_solution)(from_dof);

                // Swap again
                swapped = Moose::swapLibMeshComm(_multi_app->comm());

                solution.set(dof, from_value);
              }
            }
          }
          else // Elemental
          {
            MeshBase::const_element_iterator elem_it = mesh->local_elements_begin();
            MeshBase::const_element_iterator elem_end = mesh->local_elements_end();

            for(; elem_it != elem_end; ++elem_it)
            {
              Elem * elem = *elem_it;

              Point centroid = elem->centroid();
              Point actual_position = centroid+_multi_app->position(i);

              if(elem->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this elem
              {
                // The zero only works for LAGRANGE!
                unsigned int dof = elem->dof_number(sys_num, var_num, 0);

                // Swap back
                Moose::swapLibMeshComm(swapped);

                Real distance = 0; // Just to satisfy the last argument

                Node * nearest_node = getNearestNode(*from_mesh, actual_position, distance);

                // Assuming LAGRANGE!
                unsigned int from_dof = nearest_node->dof_number(from_sys_num, from_var_num, 0);
                Real from_value = (*serialized_solution)(from_dof);

                // Swap again
                swapped = Moose::swapLibMeshComm(_multi_app->comm());

                solution.set(dof, from_value);
              }
            }
          }

          solution.close();
          to_sys->update();

          // Swap back
          Moose::swapLibMeshComm(swapped);
        }
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblem & to_problem = *_multi_app->problem();
      MooseVariable & to_var = to_problem.getVariable(0, _to_var_name);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      NumericVector<Real> & to_solution = *to_sys.solution;

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(), "MultiAppNearestNodeTransfer only works with SerialMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      EquationSystems & to_es = to_sys.get_equation_systems();

      MeshBase * to_mesh = NULL;

      if(_displaced_source_mesh && to_problem.getDisplacedProblem())
        to_mesh = &to_problem.getDisplacedProblem()->mesh().getMesh();
      else
        to_mesh = &to_problem.mesh().getMesh();

      bool is_nodal = to_sys.variable_type(to_var_num) == FEType();

      unsigned int n_nodes = to_mesh->n_nodes();
      unsigned int n_elems = to_mesh->n_elem();

      ///// All of the following are indexed off to_node->id() or to_elem->id() /////

      // Minimum distances from each node in the "to" mesh to a node in
      std::vector<Real> min_distances;

      // The node ids in the "from" mesh that this processor has found to be the minimum distances to the "to" nodes
      std::vector<unsigned int> min_nodes;

      // After the call to maxloc() this will tell us which processor actually has the minimum
      std::vector<unsigned int> min_procs;

      // The global multiapp ID that this processor found had the minimum distance node in it.
      std::vector<unsigned int> min_apps;


      if(is_nodal)
      {
        min_distances.resize(n_nodes, std::numeric_limits<Real>::max());
        min_nodes.resize(n_nodes);
        min_procs.resize(n_nodes);
        min_apps.resize(n_nodes);
      }
      else
      {
        min_distances.resize(n_elems, std::numeric_limits<Real>::max());
        min_nodes.resize(n_elems);
        min_procs.resize(n_elems);
        min_apps.resize(n_elems);
      }


      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if(!_multi_app->hasLocalApp(i))
          continue;

        MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

        FEProblem & from_problem = *_multi_app->appProblem(i);
        MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();

        // Only works with a serialized mesh to transfer from!
        mooseAssert(from_sys.get_mesh().is_serial(), "MultiAppNearestNodeTransfer only works with SerialMesh!");

        unsigned int from_var_num = from_sys.variable_number(from_var.name());

        EquationSystems & from_es = from_sys.get_equation_systems();

        MeshBase * from_mesh = NULL;

        if(_displaced_source_mesh && from_problem.getDisplacedProblem())
          from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
        else
          from_mesh = &from_problem.mesh().getMesh();

        MeshTools::BoundingBox app_box = MeshTools::processor_bounding_box(*from_mesh, libMesh::processor_id());
        Point app_position = _multi_app->position(i);

        Moose::swapLibMeshComm(swapped);

        if(is_nodal)
        {
          MeshBase::const_node_iterator to_node_it = to_mesh->nodes_begin();
          MeshBase::const_node_iterator to_node_end = to_mesh->nodes_end();

          for(; to_node_it != to_node_end; ++to_node_it)
          {
            Node * to_node = *to_node_it;
            unsigned int to_node_id = to_node->id();

            Real current_distance;

            MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
            Node * nearest_node = getNearestNode(*from_mesh, *to_node-app_position, current_distance);
            Moose::swapLibMeshComm(swapped);

            if(current_distance < min_distances[to_node->id()])
            {
              min_distances[to_node_id] = current_distance;
              min_nodes[to_node_id] = nearest_node->id();
              min_apps[to_node_id] = i;
            }
          }
        }
        else // Elemental
        {
          MeshBase::const_element_iterator to_elem_it = to_mesh->elements_begin();
          MeshBase::const_element_iterator to_elem_end = to_mesh->elements_end();

          for(; to_elem_it != to_elem_end; ++to_elem_it)
          {
            Elem * to_elem = *to_elem_it;
            unsigned int to_elem_id = to_elem->id();

            Point actual_position = to_elem->centroid()-app_position;

            Real current_distance;

            MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());
            Node * nearest_node = getNearestNode(*from_mesh, actual_position, current_distance);
            Moose::swapLibMeshComm(swapped);

            if(current_distance < min_distances[to_elem->id()])
            {
              min_distances[to_elem_id] = current_distance;
              min_nodes[to_elem_id] = nearest_node->id();
              min_apps[to_elem_id] = i;
            }
          }
        }
      }


      // We're going to need serialized solution vectors for each app
      // We could try to only do it for the apps that have mins in them...
      // but it's tough because this is a collective operation... so that would have to be coordinated
      std::vector<NumericVector<Number> *> serialized_from_solutions(_multi_app->numGlobalApps());

      // Swap
      MPI_Comm swapped = Moose::swapLibMeshComm(_multi_app->comm());

      for(unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      {
        if(!_multi_app->hasLocalApp(i))
          continue;

        FEProblem & from_problem = *_multi_app->appProblem(i);
        MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
        SystemBase & from_system_base = from_var.sys();

        System & from_sys = from_system_base.system();

        //Create a serialized version of the solution vector
        serialized_from_solutions[i] = NumericVector<Number>::build().release();
        serialized_from_solutions[i]->init(from_sys.n_dofs(), false, SERIAL);

        // Need to pull down a full copy of this vector on every processor so we can get values in parallel
        from_sys.solution->localize(*serialized_from_solutions[i]);
      }

      // Swap back
      Moose::swapLibMeshComm(swapped);

      // We've found the nearest nodes for this processor.  We need to see which processor _actually_ found the nearest though
      Parallel::minloc(min_distances, min_procs);

      // Now loop through min_procs and see if _this_ processor had the actual minimum for any nodes.
      // If it did then we're going to go get the value from that nearest node and transfer its value
      unsigned int proc_id = libMesh::processor_id();

      for(unsigned int j=0; j<min_procs.size(); j++)
      {
        if(min_procs[j] == proc_id) // This means that this processor really did find the minumum so we need to transfer the value
        {
          // The zero only works for LAGRANGE!
          unsigned int to_dof = 0;

          if(is_nodal)
          {
            Node & to_node = to_mesh->node(j);
            to_dof = to_node.dof_number(to_sys_num, to_var_num, 0);
          }
          else
          {
            Elem & to_elem = *to_mesh->elem(j);
            to_dof = to_elem.dof_number(to_sys_num, to_var_num, 0);
          }

          // The app that has the nearest node in it
          unsigned int from_app_num = min_apps[j];

          mooseAssert(_multi_app->hasLocalApp(from_app_num), "Something went very wrong!");

          // Swap
          swapped = Moose::swapLibMeshComm(_multi_app->comm());

          FEProblem & from_problem = *_multi_app->appProblem(from_app_num);
          MooseVariable & from_var = from_problem.getVariable(0, _from_var_name);
          SystemBase & from_system_base = from_var.sys();

          System & from_sys = from_system_base.system();
          unsigned int from_sys_num = from_sys.number();

          unsigned int from_var_num = from_sys.variable_number(from_var.name());

          EquationSystems & from_es = from_sys.get_equation_systems();

          MeshBase * from_mesh = NULL;

          if(_displaced_source_mesh && from_problem.getDisplacedProblem())
            from_mesh = &from_problem.getDisplacedProblem()->mesh().getMesh();
          else
            from_mesh = &from_problem.mesh().getMesh();

          Node & from_node = from_mesh->node(min_nodes[j]);

          // Assuming LAGRANGE!
          unsigned int from_dof = from_node.dof_number(from_sys_num, from_var_num, 0);
          Real from_value = (*serialized_from_solutions[from_app_num])(from_dof);

          // Swap back
          Moose::swapLibMeshComm(swapped);

          to_solution.set(to_dof, from_value);
        }
      }

      to_solution.close();
      to_sys.update();

      break;
      }
  }

  std::cout<<"Finished NearestNodeTransfer!"<<std::endl;
}

Node * MultiAppNearestNodeTransfer::getNearestNode(MeshBase & mesh, const Point & p, Real & distance)
{
  MeshBase::const_node_iterator node_it = mesh.nodes_begin();
  MeshBase::const_node_iterator node_end = mesh.nodes_end();

  distance = std::numeric_limits<Real>::max();
  Node * nearest = NULL;

  for(; node_it != node_end; ++node_it)
  {
    Node & node = *(*node_it);
    Real current_distance = (p-node).size();

    if(current_distance < distance)
    {
      distance = current_distance;
      nearest = &node;
    }
  }

  return nearest;
}


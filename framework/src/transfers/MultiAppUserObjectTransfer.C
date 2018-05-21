//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppUserObjectTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "UserObject.h"

// libMesh
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", MultiAppUserObjectTransfer);

template <>
InputParameters
validParams<MultiAppUserObjectTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<AuxVariableName>(
      "variable", "The auxiliary variable to store the transferred values in.");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject you want to transfer values from.  Note: This might be a "
      "UserObject from your MultiApp's input file!");

  return params;
}

MultiAppUserObjectTransfer::MultiAppUserObjectTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _to_var_name(getParam<AuxVariableName>("variable")),
    _user_object_name(getParam<UserObjectName>("user_object"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppUserObjectTransfer");
}

void
MultiAppUserObjectTransfer::initialSetup()
{
  if (_direction == TO_MULTIAPP)
    variableIntegrityCheck(_to_var_name);
}

void
MultiAppUserObjectTransfer::execute()
{
  _console << "Beginning MultiAppUserObjectTransfer " << name() << std::endl;

  switch (_direction)
  {
    case TO_MULTIAPP:
    {
      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (_multi_app->hasLocalApp(i))
        {
          Moose::ScopedCommSwapper swapper(_multi_app->comm());

          // Loop over the master nodes and set the value of the variable
          System * to_sys = find_sys(_multi_app->appProblemBase(i).es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = _multi_app->appTransferVector(i, _to_var_name);

          MeshBase * mesh = NULL;

          if (_displaced_target_mesh && _multi_app->appProblemBase(i).getDisplacedProblem())
          {
            mesh = &_multi_app->appProblemBase(i).getDisplacedProblem()->mesh().getMesh();
          }
          else
            mesh = &_multi_app->appProblemBase(i).mesh().getMesh();

          bool is_nodal = to_sys->variable_type(var_num).family == LAGRANGE;

          const UserObject & user_object =
              _multi_app->problemBase().getUserObjectBase(_user_object_name);

          if (is_nodal)
          {
            for (auto & node : mesh->local_node_ptr_range())
            {
              if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                swapper.forceSwap();
                Real from_value = user_object.spatialValue(*node + _multi_app->position(i));
                swapper.forceSwap();

                solution.set(dof, from_value);
              }
            }
          }
          else // Elemental
          {
            for (auto & elem : as_range(mesh->local_elements_begin(), mesh->local_elements_end()))
            {
              Point centroid = elem->centroid();

              if (elem->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this elem
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = elem->dof_number(sys_num, var_num, 0);

                swapper.forceSwap();
                Real from_value = user_object.spatialValue(centroid + _multi_app->position(i));
                swapper.forceSwap();

                solution.set(dof, from_value);
              }
            }
          }

          solution.close();
          to_sys->update();
        }
      }

      break;
    }
    case FROM_MULTIAPP:
    {
      FEProblemBase & to_problem = _multi_app->problemBase();
      MooseVariableFEBase & to_var = to_problem.getVariable(
          0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(),
                  "MultiAppUserObjectTransfer only works with ReplicatedMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      _console << "Transferring to: " << to_var.name() << std::endl;

      // EquationSystems & to_es = to_sys.get_equation_systems();

      // Create a serialized version of the solution vector
      NumericVector<Number> * to_solution = to_sys.solution.get();

      MeshBase * to_mesh = NULL;

      if (_displaced_target_mesh && to_problem.getDisplacedProblem())
        to_mesh = &to_problem.getDisplacedProblem()->mesh().getMesh();
      else
        to_mesh = &to_problem.mesh().getMesh();

      bool is_nodal = to_sys.variable_type(to_var_num).family == LAGRANGE;

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
      {
        if (!_multi_app->hasLocalApp(i))
          continue;

        Point app_position = _multi_app->position(i);
        BoundingBox app_box = _multi_app->getBoundingBox(i, _displaced_source_mesh);
        const UserObject & user_object = _multi_app->appUserObjectBase(i, _user_object_name);

        if (is_nodal)
        {
          for (auto & node : to_mesh->node_ptr_range())
          {
            if (node->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this node
            {
              // See if this node falls in this bounding box
              if (app_box.contains_point(*node))
              {
                dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);

                Real from_value = 0;
                {
                  Moose::ScopedCommSwapper swapper(_multi_app->comm());
                  from_value = user_object.spatialValue(*node - app_position);
                }

                to_solution->set(dof, from_value);
              }
            }
          }
        }
        else // Elemental
        {
          for (auto & elem : as_range(to_mesh->elements_begin(), to_mesh->elements_end()))
          {
            if (elem->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this elem
            {
              Point centroid = elem->centroid();

              // See if this elem falls in this bounding box
              if (app_box.contains_point(centroid))
              {
                dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, 0);

                Real from_value = 0;
                {
                  Moose::ScopedCommSwapper swapper(_multi_app->comm());
                  from_value = user_object.spatialValue(centroid - app_position);
                }

                to_solution->set(dof, from_value);
              }
            }
          }
        }
      }

      to_solution->close();
      to_sys.update();

      break;
    }
  }

  _console << "Finished MultiAppUserObjectTransfer " << name() << std::endl;
}

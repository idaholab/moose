//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppUserObjectTransfer.h"

#include <limits>

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
  InputParameters params = validParams<MultiAppFieldTransfer>();
  //  MultiAppUserObjectTransfer does not need source variable since it query values from user
  //  objects
  params.suppressParameter<std::vector<VariableName>>("source_variable");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject you want to transfer values from.  Note: This might be a "
      "UserObject from your MultiApp's input file!");
  params.addParam<bool>("all_master_nodes_contained_in_sub_app",
                        false,
                        "Set to true if every master node is mapped to a distinct point on one of "
                        "the subApps during a transfer from sub App to Master App. If master node "
                        "cannot be found within bounding boxes of any of the subApps, an error is "
                        "generated.");
  return params;
}

MultiAppUserObjectTransfer::MultiAppUserObjectTransfer(const InputParameters & parameters)
  : MultiAppFieldTransfer(parameters),
    _user_object_name(getParam<UserObjectName>("user_object")),
    _all_master_nodes_contained_in_sub_app(getParam<bool>("all_master_nodes_contained_in_sub_app"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppUserObjectTransfer");

  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() > 0)
    paramError("source_variable",
               " You should not provide any source variables since the transfer takes values from "
               "user objects ");
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

          auto & fe_type = to_sys->variable_type(var_num);
          bool is_constant = fe_type.order == CONSTANT;
          bool is_nodal = fe_type.family == LAGRANGE;

          if (fe_type.order > FIRST && !is_nodal)
            mooseError("We don't currently support second order or higher elemental variable ");

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
            std::vector<Point> points;
            for (auto & elem : as_range(mesh->local_elements_begin(), mesh->local_elements_end()))
            {
              // Skip this element if the variable has no dofs at it.
              if (elem->n_dofs(sys_num, var_num) < 1)
                continue;

              points.clear();
              // grap sample points
              // for constant shape function, we take the element centroid
              if (is_constant)
                points.push_back(elem->centroid());
              // for higher order method, we take all nodes of element
              // this works for the first order L2 Lagrange.
              else
                for (auto & node : elem->node_ref_range())
                  points.push_back(node);

              auto n_points = points.size();
              unsigned int n_comp = elem->n_comp(sys_num, var_num);
              // We assume each point corresponds to one component of elemental variable
              if (n_points != n_comp)
                mooseError(" Number of points ",
                           n_points,
                           " does not equal to number of variable components ",
                           n_comp);

              unsigned int offset = 0;
              for (auto & point : points) // If this variable has dofs at this elem
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = elem->dof_number(sys_num, var_num, offset++);

                swapper.forceSwap();
                Real from_value = user_object.spatialValue(point + _multi_app->position(i));
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

      auto & fe_type = to_sys.variable_type(to_var_num);
      bool is_constant = fe_type.order == CONSTANT;
      bool is_nodal = fe_type.family == LAGRANGE;

      if (fe_type.order > FIRST && !is_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      if (_all_master_nodes_contained_in_sub_app)
      {
        // check to see if master nodes or elements lies within any of the sub application bounding
        // boxes
        if (is_nodal)
        {
          for (auto & node : to_mesh->node_ptr_range())
          {
            if (node->n_dofs(to_sys_num, to_var_num) > 0)
            {
              unsigned int node_found_in_sub_app = 0;
              for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
              {
                if (!_multi_app->hasLocalApp(i))
                  continue;

                BoundingBox app_box = _multi_app->getBoundingBox(i, _displaced_source_mesh);

                if (app_box.contains_point(*node))
                  ++node_found_in_sub_app;
              }

              if (node_found_in_sub_app == 0)
              {
                Point n = *node;
                mooseError("MultiAppUserObjectTransfer: Master node ",
                           n,
                           " not found within the bounding box of any of the sub applications.");
              }
              else if (node_found_in_sub_app > 1)
              {
                Point n = *node;
                mooseError("MultiAppUserObjectTransfer: Master node ",
                           n,
                           " found within the bounding box of two or more sub applications.");
              }
            }
          }
        }
        else // elemental
        {
          std::vector<Point> points;
          for (auto & elem : as_range(to_mesh->elements_begin(), to_mesh->elements_end()))
          {
            // Skip this element if the variable has no dofs at it.
            if (elem->n_dofs(to_sys_num, to_var_num) < 1)
              continue;

            points.clear();
            // grap sample points
            // for constant shape function, we take the element centroid
            if (is_constant)
              points.push_back(elem->centroid());
            // for higher order method, we take all nodes of element
            // this works for the first order L2 Lagrange.
            else
              for (auto & node : elem->node_ref_range())
                points.push_back(node);

            auto n_points = points.size();
            unsigned int n_comp = elem->n_comp(to_sys_num, to_var_num);
            // We assume each point corresponds to one component of elemental variable
            if (n_points != n_comp)
              mooseError(" Number of points ",
                         n_points,
                         " does not equal to number of variable components ",
                         n_comp);

            for (auto & point : points)
            {
              unsigned int elem_found_in_sub_app = 0;

              for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
              {
                if (!_multi_app->hasLocalApp(i))
                  continue;

                BoundingBox app_box = _multi_app->getBoundingBox(i, _displaced_source_mesh);

                if (app_box.contains_point(point))
                  ++elem_found_in_sub_app;
              }

              if (elem_found_in_sub_app == 0)
                mooseError("MultiAppUserObjectTransfer: Master element with ",
                           n_points > 1 ? "node" : "centroid",
                           " at ",
                           point,
                           " not found within the bounding box of any of the sub applications.");

              else if (elem_found_in_sub_app > 1)
                mooseError("MultiAppUserObjectTransfer: Master element with ",
                           n_points > 1 ? "node" : "centroid",
                           " at ",
                           point,
                           " found within the bounding box of two or more sub applications.");
            }
          }
        }
      }

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

                if (from_value == std::numeric_limits<Real>::infinity())
                {
                  Point n = *node;
                  mooseError("MultiAppUserObjectTransfer: Point corresponding to master node at (",
                             n,
                             ") not found in the sub application.");
                }
                to_solution->set(dof, from_value);
              }
            }
          }
        }
        else // Elemental
        {
          std::vector<Point> points;
          for (auto & elem : as_range(to_mesh->elements_begin(), to_mesh->elements_end()))
          {
            // Skip this element if the variable has no dofs at it.
            if (elem->n_dofs(to_sys_num, to_var_num) < 1)
              continue;

            points.clear();
            // grap sample points
            // for constant shape function, we take the element centroid
            if (is_constant)
              points.push_back(elem->centroid());
            // for higher order method, we take all nodes of element
            // this works for the first order L2 Lagrange.
            else
              for (auto & node : elem->node_ref_range())
                points.push_back(node);

            auto n_points = points.size();
            unsigned int n_comp = elem->n_comp(to_sys_num, to_var_num);
            // We assume each point corresponds to one component of elemental variable
            if (n_points != n_comp)
              mooseError(" Number of points ",
                         n_points,
                         " does not equal to number of variable components ",
                         n_comp);

            unsigned int offset = 0;
            for (auto & point : points) // If this variable has dofs at this elem
            {
              // See if this elem falls in this bounding box
              if (app_box.contains_point(point))
              {
                dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, offset++);

                Real from_value = 0;
                {
                  Moose::ScopedCommSwapper swapper(_multi_app->comm());
                  from_value = user_object.spatialValue(point - app_position);
                }

                if (from_value == std::numeric_limits<Real>::infinity())
                  mooseError(
                      "MultiAppUserObjectTransfer: Point corresponding to element's centroid (",
                      point,
                      ") not found in sub application.");

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

  postExecute();
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppUserObjectTransfer.h"
#include "MooseAppCoordTransform.h"

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

InputParameters
MultiAppUserObjectTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  //  MultiAppUserObjectTransfer does not need source variable since it query values from user
  //  objects
  params.suppressParameter<std::vector<VariableName>>("source_variable");
  params.addRequiredParam<UserObjectName>(
      "user_object",
      "The UserObject you want to transfer values from.  Note: This might be a "
      "UserObject from your MultiApp's input file!");
  params.addParam<bool>("all_parent_nodes_contained_in_sub_app",
                        false,
                        "Set to true if every parent app node is mapped to a distinct point on one "
                        "of the subApps during a transfer from sub App to Parent App. If parent app"
                        " node cannot be found within bounding boxes of any of the subApps, an "
                        " error is generated.");
  params.addDeprecatedParam<bool>(
      "all_master_nodes_contained_in_sub_app",
      "Set to true if every parent app node is mapped to a distinct point on one "
      "of the subApps during a transfer from sub App to Parent App. If parent app"
      " node cannot be found within bounding boxes of any of the subApps, an "
      " error is generated.",
      "all_master_nodes_contained_in_sub_app is deprecated. Use "
      "all_parent_nodes_contained_in_sub_app");
  params.addParam<bool>(
      "skip_bounding_box_check",
      false,
      "Skip the check if the to_elem is within the bounding box of the from_app.");
  params.addParam<std::vector<SubdomainName>>(
      "block", "The block we are transferring to (if not specified, whole domain is used).");
  params.addParam<std::vector<BoundaryName>>(
      "boundary",
      "The boundary we are transferring to (if not specified, whole domain is used unless 'block' "
      "parameter is used).");

  params.addClassDescription(
      "Samples a variable's value in the Parent app domain at the point where the MultiApp is and "
      "copies that value into a post-processor in the MultiApp");

  params.addParam<bool>("nearest_sub_app",
                        false,
                        "When True, a from_multiapp transfer will work by finding the nearest "
                        "(using the `location`) sub-app and query that for the value to transfer");
  return params;
}

MultiAppUserObjectTransfer::MultiAppUserObjectTransfer(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _user_object_name(getParam<UserObjectName>("user_object")),
    _all_parent_nodes_contained_in_sub_app(
        isParamValid("all_master_nodes_contained_in_sub_app")
            ? getParam<bool>("all_master_nodes_contained_in_sub_app")
            : getParam<bool>("all_parent_nodes_contained_in_sub_app")),
    _skip_bbox_check(getParam<bool>("skip_bounding_box_check")),
    _nearest_sub_app(getParam<bool>("nearest_sub_app"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppUserObjectTransfer");

  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() > 0)
    paramError("source_variable",
               " You should not provide any source variables since the transfer takes values from "
               "user objects ");

  if (isParamValid("block") && isParamValid("boundary"))
    mooseError(name(), ": Transfer can be either block- or boundary-restricted. Not both.");

  if (isParamValid("to_multi_app") && isParamValid("from_multi_app") &&
      getToMultiApp() != getFromMultiApp())
    paramError("to_multi_app",
               "Sibling multiapp transfer has not been implemented for this transfer.");
}

void
MultiAppUserObjectTransfer::execute()
{
  TIME_SECTION(
      "MultiAppUserObjectTransfer::execute()", 5, "Performing transfer with a user object");

  getAppInfo();

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          Moose::ScopedCommSwapper swapper(getToMultiApp()->comm());

          // Loop over the parent app nodes and set the value of the variable
          System * to_sys = find_sys(getToMultiApp()->appProblemBase(i).es(), _to_var_name);

          unsigned int sys_num = to_sys->number();
          unsigned int var_num = to_sys->variable_number(_to_var_name);

          NumericVector<Real> & solution = getToMultiApp()->appTransferVector(i, _to_var_name);

          MooseMesh * mesh = NULL;

          if (_displaced_target_mesh && getToMultiApp()->appProblemBase(i).getDisplacedProblem())
            mesh = &getToMultiApp()->appProblemBase(i).getDisplacedProblem()->mesh();
          else
            mesh = &getToMultiApp()->appProblemBase(i).mesh();

          _blk_ids.clear();
          _bnd_ids.clear();
          if (isParamValid("block"))
          {
            const std::vector<SubdomainName> & blocks =
                getParam<std::vector<SubdomainName>>("block");
            for (const auto & b : blocks)
              if (!MooseMeshUtils::hasSubdomainName(*mesh, b))
                paramError("block", "The block '", b, "' was not found in the mesh");

            std::vector<SubdomainID> ids = mesh->getSubdomainIDs(blocks);
            _blk_ids.insert(ids.begin(), ids.end());
          }
          else if (isParamValid("boundary"))
          {
            const std::vector<BoundaryName> & boundary_names =
                getParam<std::vector<BoundaryName>>("boundary");
            for (const auto & b : boundary_names)
              if (!MooseMeshUtils::hasBoundaryName(*mesh, b))
                paramError("boundary", "The boundary '", b, "' was not found in the mesh");

            std::vector<BoundaryID> ids = mesh->getBoundaryIDs(boundary_names, true);
            _bnd_ids.insert(ids.begin(), ids.end());
          }

          auto & fe_type = to_sys->variable_type(var_num);
          bool is_constant = fe_type.order == CONSTANT;
          bool is_nodal = fe_type.family == LAGRANGE;

          if (fe_type.order > FIRST && !is_nodal)
            mooseError("We don't currently support second order or higher elemental variable ");

          const UserObject & user_object =
              getToMultiApp()->problemBase().getUserObjectBase(_user_object_name);
          mooseAssert(_from_transforms.size() == 1, "This should have size 1");
          const auto & from_transform = *_from_transforms[0];
          const auto & to_transform = *_to_transforms[i];

          if (is_nodal)
          {
            for (auto & node : mesh->getMesh().local_node_ptr_range())
            {
              if (blockRestricted() && !hasBlocks(mesh, node))
                continue;

              if (boundaryRestricted() && !isBoundaryNode(mesh, node))
                continue;

              if (node->n_dofs(sys_num, var_num) > 0) // If this variable has dofs at this node
              {
                // The zero only works for LAGRANGE!
                dof_id_type dof = node->dof_number(sys_num, var_num, 0);

                swapper.forceSwap();
                Real from_value =
                    user_object.spatialValue(from_transform.mapBack(to_transform(*node)));
                swapper.forceSwap();

                solution.set(dof, from_value);
              }
            }
          }
          else // Elemental
          {
            std::vector<Point> points;
            for (auto & elem : as_range(mesh->getMesh().local_elements_begin(),
                                        mesh->getMesh().local_elements_end()))
            {
              if (blockRestricted() && !hasBlocks(elem))
                continue;

              if (boundaryRestricted() && !isBoundaryElem(mesh, elem))
                continue;

              // Skip this element if the variable has no dofs at it.
              if (elem->n_dofs(sys_num, var_num) < 1)
                continue;

              points.clear();
              // grap sample points
              // for constant shape function, we take the element centroid
              if (is_constant)
                points.push_back(elem->vertex_average());
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
                dof_id_type dof = elem->dof_number(sys_num, var_num, offset++);

                swapper.forceSwap();
                Real from_value =
                    user_object.spatialValue(from_transform.mapBack(to_transform(point)));
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
      FEProblemBase & to_problem = getFromMultiApp()->problemBase();
      mooseAssert(_to_transforms.size() == 1, "This should only be size one");
      const auto & to_transform = *_to_transforms[0];
      MooseVariableFEBase & to_var = to_problem.getVariable(
          0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);
      SystemBase & to_system_base = to_var.sys();

      System & to_sys = to_system_base.system();

      unsigned int to_sys_num = to_sys.number();

      // Only works with a serialized mesh to transfer to!
      mooseAssert(to_sys.get_mesh().is_serial(),
                  "MultiAppUserObjectTransfer only works with ReplicatedMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      // EquationSystems & to_es = to_sys.get_equation_systems();

      // Create a serialized version of the solution vector
      NumericVector<Number> * to_solution = to_sys.solution.get();

      MooseMesh * to_mesh = NULL;

      if (_displaced_target_mesh && to_problem.getDisplacedProblem())
        to_mesh = &to_problem.getDisplacedProblem()->mesh();
      else
        to_mesh = &to_problem.mesh();

      _blk_ids.clear();
      _bnd_ids.clear();
      if (isParamValid("block"))
      {
        const std::vector<SubdomainName> & blocks = getParam<std::vector<SubdomainName>>("block");
        for (const auto & b : blocks)
          if (!MooseMeshUtils::hasSubdomainName(*to_mesh, b))
            paramError("block", "The block '", b, "' was not found in the mesh");

        std::vector<SubdomainID> ids = to_mesh->getSubdomainIDs(blocks);
        _blk_ids.insert(ids.begin(), ids.end());
      }
      else if (isParamValid("boundary"))
      {
        const std::vector<BoundaryName> & boundary_names =
            getParam<std::vector<BoundaryName>>("boundary");
        for (const auto & b : boundary_names)
          if (!MooseMeshUtils::hasBoundaryName(*to_mesh, b))
            paramError("boundary", "The boundary '", b, "' was not found in the mesh");

        std::vector<BoundaryID> ids = to_mesh->getBoundaryIDs(boundary_names, true);
        _bnd_ids.insert(ids.begin(), ids.end());
      }

      auto & fe_type = to_sys.variable_type(to_var_num);
      bool is_constant = fe_type.order == CONSTANT;
      bool is_nodal = fe_type.family == LAGRANGE;

      if (fe_type.order > FIRST && !is_nodal)
        mooseError("We don't currently support second order or higher elemental variable ");

      if (_all_parent_nodes_contained_in_sub_app)
      {
        // check to see if parent app nodes or elements lies within any of the sub application
        // bounding boxes
        if (is_nodal)
        {
          for (auto & node : to_mesh->getMesh().node_ptr_range())
          {
            if (blockRestricted() && !hasBlocks(to_mesh, node))
              continue;

            if (boundaryRestricted() && !isBoundaryNode(to_mesh, node))
              continue;

            if (node->n_dofs(to_sys_num, to_var_num) > 0)
            {
              const auto transformed_node = to_transform(*node);

              unsigned int node_found_in_sub_app = 0;
              for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
              {
                if (!getFromMultiApp()->hasLocalApp(i))
                  continue;

                BoundingBox app_box = getFromMultiApp()->getBoundingBox(
                    i, _displaced_source_mesh, _from_transforms[i].get());

                if (app_box.contains_point(transformed_node))
                  ++node_found_in_sub_app;
              }

              if (node_found_in_sub_app == 0)
                mooseError("MultiAppUserObjectTransfer: Parent app node ",
                           transformed_node,
                           " not found within the bounding box of any of the sub applications.");
              else if (node_found_in_sub_app > 1)
                mooseError("MultiAppUserObjectTransfer: Parent app node ",
                           transformed_node,
                           " found within the bounding box of two or more sub applications.");
            }
          }
        }
        else // elemental
        {
          std::vector<Point> points;
          for (auto & elem :
               as_range(to_mesh->getMesh().elements_begin(), to_mesh->getMesh().elements_end()))
          {
            if (blockRestricted() && !hasBlocks(elem))
              continue;

            if (boundaryRestricted() && !isBoundaryElem(to_mesh, elem))
              continue;

            // Skip this element if the variable has no dofs at it.
            if (elem->n_dofs(to_sys_num, to_var_num) < 1)
              continue;

            points.clear();
            // grap sample points
            // for constant shape function, we take the element centroid
            if (is_constant)
              points.push_back(to_transform(elem->vertex_average()));
            // for higher order method, we take all nodes of element
            // this works for the first order L2 Lagrange.
            else
              for (auto & node : elem->node_ref_range())
                points.push_back(to_transform(node));

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

              for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
              {
                if (!getFromMultiApp()->hasLocalApp(i))
                  continue;

                BoundingBox app_box = getFromMultiApp()->getBoundingBox(
                    i, _displaced_source_mesh, _from_transforms[i].get());

                if (app_box.contains_point(point))
                  ++elem_found_in_sub_app;
              }

              if (elem_found_in_sub_app == 0)
                mooseError("MultiAppUserObjectTransfer: Parent app element with ",
                           n_points > 1 ? "node" : "centroid",
                           " at ",
                           point,
                           " not found within the bounding box of any of the sub applications.");

              else if (elem_found_in_sub_app > 1)
                mooseError("MultiAppUserObjectTransfer: Parent app element with ",
                           n_points > 1 ? "node" : "centroid",
                           " at ",
                           point,
                           " found within the bounding box of two or more sub applications.");
            }
          }
        }
      }

      if (is_nodal)
      {
        for (auto & node : to_mesh->getMesh().node_ptr_range())
        {
          if (blockRestricted() && !hasBlocks(to_mesh, node))
            continue;

          if (boundaryRestricted() && !isBoundaryNode(to_mesh, node))
            continue;

          if (node->n_dofs(to_sys_num, to_var_num) > 0) // If this variable has dofs at this node
          {
            const auto transformed_node = to_transform(*node);
            const auto sub_app = findSubAppToTransferFrom(transformed_node);

            // Check to see if a sub-app was found
            if (sub_app == static_cast<unsigned int>(-1))
              continue;

            const auto & from_transform = *_from_transforms[sub_app];
            const auto & user_object = _multi_app->appUserObjectBase(sub_app, _user_object_name);

            dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);

            Real from_value = 0;
            {
              Moose::ScopedCommSwapper swapper(getFromMultiApp()->comm());
              from_value = user_object.spatialValue(from_transform.mapBack(transformed_node));
            }

            if (from_value == std::numeric_limits<Real>::infinity())
              mooseError("MultiAppUserObjectTransfer: Point corresponding to parent app node at (",
                         transformed_node,
                         ") not found in the sub application.");
            to_solution->set(dof, from_value);
          }
        }
      }
      else // Elemental
      {
        std::vector<Point> points;
        for (auto & elem :
             as_range(to_mesh->getMesh().elements_begin(), to_mesh->getMesh().elements_end()))
        {
          if (blockRestricted() && !hasBlocks(elem))
            continue;

          if (boundaryRestricted() && !isBoundaryElem(to_mesh, elem))
            continue;

          // Skip this element if the variable has no dofs at it.
          if (elem->n_dofs(to_sys_num, to_var_num) < 1)
            continue;

          points.clear();
          // grap sample points
          // for constant shape function, we take the element centroid
          if (is_constant)
            points.push_back(to_transform(elem->vertex_average()));
          // for higher order method, we take all nodes of element
          // this works for the first order L2 Lagrange.
          else
            for (auto & node : elem->node_ref_range())
              points.push_back(to_transform(node));

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
            const auto sub_app = findSubAppToTransferFrom(point);

            // Check to see if a sub-app was found
            if (sub_app == static_cast<unsigned int>(-1))
              continue;

            const auto & from_transform = *_from_transforms[sub_app];
            const auto & user_object =
                getFromMultiApp()->appUserObjectBase(sub_app, _user_object_name);

            dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, offset++);

            Real from_value = 0;
            {
              Moose::ScopedCommSwapper swapper(getFromMultiApp()->comm());
              from_value = user_object.spatialValue(from_transform.mapBack(to_transform(point)));
            }

            if (from_value == std::numeric_limits<Real>::infinity())
              mooseError("MultiAppUserObjectTransfer: Point corresponding to element's centroid (",
                         point,
                         ") not found in sub application.");

            to_solution->set(dof, from_value);
          }
        }
      }

      to_solution->close();
      to_sys.update();

      break;
    }
  }

  postExecute();
}

bool
MultiAppUserObjectTransfer::blockRestricted() const
{
  return !_blk_ids.empty();
}

bool
MultiAppUserObjectTransfer::boundaryRestricted() const
{
  return !_bnd_ids.empty();
}

bool
MultiAppUserObjectTransfer::hasBlocks(const Elem * elem) const
{
  return _blk_ids.find(elem->subdomain_id()) != _blk_ids.end();
}

bool
MultiAppUserObjectTransfer::hasBlocks(const MooseMesh * mesh, const Node * node) const
{
  const std::set<SubdomainID> & node_blk_ids = mesh->getNodeBlockIds(*node);
  std::set<SubdomainID> u;
  std::set_intersection(_blk_ids.begin(),
                        _blk_ids.end(),
                        node_blk_ids.begin(),
                        node_blk_ids.end(),
                        std::inserter(u, u.begin()));
  return !u.empty();
}

bool
MultiAppUserObjectTransfer::isBoundaryNode(const MooseMesh * mesh, const Node * node) const
{
  for (auto & bid : _bnd_ids)
    if (mesh->isBoundaryNode(node->id(), bid))
      return true;
  return false;
}

bool
MultiAppUserObjectTransfer::isBoundaryElem(const MooseMesh * mesh, const Elem * elem) const
{
  for (auto & bid : _bnd_ids)
    if (mesh->isBoundaryElem(elem->id(), bid))
      return true;
  return false;
}

unsigned int
MultiAppUserObjectTransfer::findSubAppToTransferFrom(const Point & p)
{
  // Just find the nearest app to this point
  if (_nearest_sub_app)
  {
    unsigned int closest_app = 0;
    Real closest_distance = std::numeric_limits<Real>::max();

    mooseAssert(_multi_app->numGlobalApps() > 0, "No Multiapps To Transfer From");

    for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
    {
      // Obtain the possibly transformed app position by querying the transform with the origin
      const auto app_position = _multi_app->runningInPosition()
                                    ? (*_from_transforms[i])(_multi_app->position(i))
                                    : (*_from_transforms[i])(Point(0));

      auto distance = (p - app_position).norm();

      if (distance < closest_distance)
      {
        closest_app = i;
        closest_distance = distance;
      }
    }

    // We can only get the value if we have this app
    // otherwise - another processor will set it
    if (_multi_app->hasLocalApp(closest_app))
      return closest_app;
    else
      return -1;
  }

  // Find the app that contains this point...

  // This loop counts _down_ so that it can preserve legacy behavior of the
  // last sub-app "winning" to be able to set the value at this point
  for (int i = _multi_app->numGlobalApps() - 1; i >= 0; i--)
  {
    if (!_multi_app->hasLocalApp(i))
      continue;

    BoundingBox app_box =
        _multi_app->getBoundingBox(i, _displaced_source_mesh, _from_transforms[i].get());

    if (_skip_bbox_check || app_box.contains_point(p))
      return static_cast<unsigned int>(i);
  }

  return -1;
}

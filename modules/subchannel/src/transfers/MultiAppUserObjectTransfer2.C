#include "MultiAppUserObjectTransfer2.h"

#include <limits>

#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "UserObject.h"

#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/mesh_function.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("SubChannelApp", MultiAppUserObjectTransfer2);

InputParameters
MultiAppUserObjectTransfer2::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  //  MultiAppUserObjectTransfer2 does not need source variable since it query values from user
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
      "Samples a variable's value in the Master domain at the point where the MultiApp is and "
      "copies that value into a post-processor in the MultiApp");

  return params;
}

MultiAppUserObjectTransfer2::MultiAppUserObjectTransfer2(const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _user_object_name(getParam<UserObjectName>("user_object")),
    _all_master_nodes_contained_in_sub_app(getParam<bool>("all_master_nodes_contained_in_sub_app")),
    _skip_bbox_check(getParam<bool>("skip_bounding_box_check"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppUserObjectTransfer2");

  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() > 0)
    paramError("source_variable",
               " You should not provide any source variables since the transfer takes values from "
               "user objects ");

  if (isParamValid("block") && isParamValid("boundary"))
    mooseError(name(), ": Transfer can be either block- or boundary-restricted. Not both.");
}

void
MultiAppUserObjectTransfer2::execute()
{
  _console << "Beginning MultiAppUserObjectTransfer2 " << name() << std::endl;

  switch (_current_direction)
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

          MooseMesh * mesh = NULL;

          if (_displaced_target_mesh && _multi_app->appProblemBase(i).getDisplacedProblem())
            mesh = &_multi_app->appProblemBase(i).getDisplacedProblem()->mesh();
          else
            mesh = &_multi_app->appProblemBase(i).mesh();

          _blk_ids.clear();
          _bnd_ids.clear();
          if (isParamValid("block"))
          {
            const std::vector<SubdomainName> & blocks =
                getParam<std::vector<SubdomainName>>("block");
            std::vector<SubdomainID> ids = mesh->getSubdomainIDs(blocks);
            _blk_ids.insert(ids.begin(), ids.end());
          }
          else if (isParamValid("boundary"))
          {
            const std::vector<BoundaryName> & boundary_names =
                getParam<std::vector<BoundaryName>>("boundary");
            std::vector<BoundaryID> ids = mesh->getBoundaryIDs(boundary_names, true);
            _bnd_ids.insert(ids.begin(), ids.end());
          }

          auto & fe_type = to_sys->variable_type(var_num);
          bool is_constant = fe_type.order == CONSTANT;
          bool is_nodal = fe_type.family == LAGRANGE;

          if (fe_type.order > FIRST && !is_nodal)
            mooseError("We don't currently support second order or higher elemental variable ");

          const UserObject & user_object =
              _multi_app->problemBase().getUserObjectBase(_user_object_name);

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
                Point pt = *node;
                std::swap(pt(1), pt(2));
                pt += _multi_app->position(i);
                Real from_value = user_object.spatialValue(pt);
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
                Point pt = point;
                std::swap(pt(1), pt(2));
                pt += _multi_app->position(i);
                Real from_value = user_object.spatialValue(pt);
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
                  "MultiAppUserObjectTransfer2 only works with ReplicatedMesh!");

      unsigned int to_var_num = to_sys.variable_number(to_var.name());

      _console << "Transferring to: " << to_var.name() << std::endl;

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
        std::vector<SubdomainID> ids = to_mesh->getSubdomainIDs(blocks);
        _blk_ids.insert(ids.begin(), ids.end());
      }
      else if (isParamValid("boundary"))
      {
        const std::vector<BoundaryName> & boundary_names =
            getParam<std::vector<BoundaryName>>("boundary");
        std::vector<BoundaryID> ids = to_mesh->getBoundaryIDs(boundary_names, true);
        _bnd_ids.insert(ids.begin(), ids.end());
      }

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
          for (auto & node : to_mesh->getMesh().node_ptr_range())
          {
            if (blockRestricted() && !hasBlocks(to_mesh, node))
              continue;

            if (boundaryRestricted() && !isBoundaryNode(to_mesh, node))
              continue;

            if (node->n_dofs(to_sys_num, to_var_num) > 0)
            {
              unsigned int node_found_in_sub_app = 0;
              for (unsigned int i = 0; i < _multi_app->numGlobalApps(); i++)
              {
                if (!_multi_app->hasLocalApp(i))
                  continue;

                BoundingBox app_box = getBoundingBox(i, _displaced_source_mesh);
                Point pt = *node;
                if (app_box.contains_point(pt))
                  ++node_found_in_sub_app;
              }

              if (node_found_in_sub_app == 0)
              {
                Point n = *node;
                mooseError("MultiAppUserObjectTransfer2: Master node ",
                           n,
                           " not found within the bounding box of any of the sub applications.");
              }
              else if (node_found_in_sub_app > 1)
              {
                Point n = *node;
                mooseError("MultiAppUserObjectTransfer2: Master node ",
                           n,
                           " found within the bounding box of two or more sub applications.");
              }
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
              points.push_back(elem->vertex_average());
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

                Point pt = point;
                if (app_box.contains_point(pt))
                  ++elem_found_in_sub_app;
              }

              if (elem_found_in_sub_app == 0)
                mooseError("MultiAppUserObjectTransfer2: Master element with ",
                           n_points > 1 ? "node" : "centroid",
                           " at ",
                           point,
                           " not found within the bounding box of any of the sub applications.");

              else if (elem_found_in_sub_app > 1)
                mooseError("MultiAppUserObjectTransfer2: Master element with ",
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
        BoundingBox app_box = getBoundingBox(i, _displaced_source_mesh);
        const UserObject & user_object = _multi_app->appUserObjectBase(i, _user_object_name);

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
              // See if this node falls in this bounding box
              Point pt = *node;
              if (_skip_bbox_check || app_box.contains_point(pt))
              {
                dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);

                Real from_value = 0;
                {
                  Moose::ScopedCommSwapper swapper(_multi_app->comm());
                  Point pt = *node;
                  pt -= app_position;
                  std::swap(pt(1), pt(2));
                  from_value = user_object.spatialValue(pt);
                }

                if (from_value == std::numeric_limits<Real>::infinity())
                {
                  Point n = *node;
                  mooseError("MultiAppUserObjectTransfer2: Point corresponding to master node at (",
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
              points.push_back(elem->vertex_average());
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
              if (_skip_bbox_check || app_box.contains_point(point))
              {
                dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, offset++);

                Real from_value = 0;
                {
                  Moose::ScopedCommSwapper swapper(_multi_app->comm());
                  Point pt = point;
                  pt -= app_position;
                  std::swap(pt(1), pt(2));
                  from_value = user_object.spatialValue(pt);
                }

                if (from_value == std::numeric_limits<Real>::infinity())
                  mooseError(
                      "MultiAppUserObjectTransfer2: Point corresponding to element's centroid (",
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

  _console << "Finished MultiAppUserObjectTransfer2 " << name() << std::endl;

  postExecute();
}

bool
MultiAppUserObjectTransfer2::blockRestricted() const
{
  return !_blk_ids.empty();
}

bool
MultiAppUserObjectTransfer2::boundaryRestricted() const
{
  return !_bnd_ids.empty();
}

bool
MultiAppUserObjectTransfer2::hasBlocks(const Elem * elem) const
{
  return _blk_ids.find(elem->subdomain_id()) != _blk_ids.end();
}

bool
MultiAppUserObjectTransfer2::hasBlocks(const MooseMesh * mesh, const Node * node) const
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
MultiAppUserObjectTransfer2::isBoundaryNode(const MooseMesh * mesh, const Node * node) const
{
  for (auto & bid : _bnd_ids)
    if (mesh->isBoundaryNode(node->id(), bid))
      return true;
  return false;
}

bool
MultiAppUserObjectTransfer2::isBoundaryElem(const MooseMesh * mesh, const Elem * elem) const
{
  for (auto & bid : _bnd_ids)
    if (mesh->isBoundaryElem(elem->id(), bid))
      return true;
  return false;
}

BoundingBox
MultiAppUserObjectTransfer2::getBoundingBox(unsigned int app, bool displaced_mesh)
{
  FEProblemBase & fe_problem_base = _multi_app->appProblemBase(app);
  MooseMesh & mesh = (displaced_mesh && fe_problem_base.getDisplacedProblem().get() != NULL)
                         ? fe_problem_base.getDisplacedProblem()->mesh()
                         : fe_problem_base.mesh();

  BoundingBox bbox;
  {
    Moose::ScopedCommSwapper swapper(_multi_app->comm());
    bbox = MeshTools::create_bounding_box(mesh);
  }

  Point min = bbox.min();
  std::swap(min(1), min(2));
  // TODO: turn this into a parameter
  min -= Point(0.001, 0.001, 0.001);
  Point max = bbox.max();
  std::swap(max(1), max(2));
  // TODO: turn this into a parameter
  max += Point(0.001, 0.001, 0.001);

  Point p = _multi_app->position(app);
  min += p;
  max += p;

  return BoundingBox(min, max);
}

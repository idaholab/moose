//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiAppGeometricInterpolationTransfer.h"

// MOOSE includes
#include "DisplacedProblem.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include "MooseTypes.h"
#include "MooseVariableFE.h"
#include "MultiApp.h"
#include "MooseAppCoordTransform.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"
#include "libmesh/radial_basis_interpolation.h"

using namespace libMesh;

registerMooseObject("MooseApp", MultiAppGeometricInterpolationTransfer);
registerMooseObjectRenamed("MooseApp",
                           MultiAppInterpolationTransfer,
                           "12/31/2023 24:00",
                           MultiAppGeometricInterpolationTransfer);

InputParameters
MultiAppGeometricInterpolationTransfer::validParams()
{
  InputParameters params = MultiAppConservativeTransfer::validParams();
  params.addClassDescription(
      "Transfers the value to the target domain from a combination/interpolation of the values on "
      "the nearest nodes in the source domain, using coefficients based on the distance to each "
      "node.");
  params.addParam<unsigned int>(
      "num_points", 3, "The number of nearest points to use for interpolation.");
  params.addParam<Real>(
      "power", 2, "The polynomial power to use for calculation of the decay in the interpolation.");

  MooseEnum interp_type("inverse_distance radial_basis", "inverse_distance");
  params.addParam<MooseEnum>("interp_type", interp_type, "The algorithm to use for interpolation.");

  params.addParam<Real>("radius",
                        -1,
                        "Radius to use for radial_basis interpolation.  If negative "
                        "then the radius is taken as the max distance between "
                        "points.");
  params.addParam<Real>(
      "shrink_gap_width",
      0,
      "gap width with which we want to temporarily shrink mesh in transfering solution");

  MooseEnum shrink_type("SOURCE TARGET", "SOURCE");
  params.addParam<MooseEnum>("shrink_mesh", shrink_type, "Which mesh we want to shrink");

  params.addParam<std::vector<SubdomainName>>(
      "exclude_gap_blocks",
      {},
      "Gap subdomains we want to exclude when constructing/using virtually translated points");

  params.addParam<Real>("distance_tol",
                        1e-10,
                        "If the distance between two points is smaller than distance_tol, two "
                        "points will be considered as identical");

  return params;
}

MultiAppGeometricInterpolationTransfer::MultiAppGeometricInterpolationTransfer(
    const InputParameters & parameters)
  : MultiAppConservativeTransfer(parameters),
    _num_points(getParam<unsigned int>("num_points")),
    _power(getParam<Real>("power")),
    _interp_type(getParam<MooseEnum>("interp_type")),
    _radius(getParam<Real>("radius")),
    _shrink_gap_width(getParam<Real>("shrink_gap_width")),
    _shrink_mesh(getParam<MooseEnum>("shrink_mesh")),
    _exclude_gap_blocks(getParam<std::vector<SubdomainName>>("exclude_gap_blocks")),
    _distance_tol(getParam<Real>("distance_tol"))
{
  // This transfer does not work with DistributedMesh
  _fe_problem.mesh().errorIfDistributedMesh("MultiAppGeometricInterpolationTransfer");

  if (_to_var_names.size() != 1)
    paramError("variable", " Support single to-variable only ");

  if (_from_var_names.size() != 1)
    paramError("source_variable", " Support single from-variable only ");
}

void
MultiAppGeometricInterpolationTransfer::subdomainIDsNode(MooseMesh & mesh,
                                                         const Node & node,
                                                         std::set<subdomain_id_type> & subdomainids)
{
  // We need this map to figure out to which subdomains a given mesh point is attached
  // We can not make mesh const here because we may need to create a node-to-elems map
  // if it does not exists
  auto & node_to_elem = mesh.nodeToElemMap();
  auto node_to_elem_pair = node_to_elem.find(node.id());

  if (node_to_elem_pair == node_to_elem.end())
    mooseError("Can not find elements for node ", node.id());

  subdomainids.clear();
  // Add all subdomain IDs that are attached to node
  for (auto element : node_to_elem_pair->second)
  {
    auto & elem = mesh.getMesh().elem_ref(element);
    auto subdomain = elem.subdomain_id();

    subdomainids.insert(subdomain);
  }
}

void
MultiAppGeometricInterpolationTransfer::fillSourceInterpolationPoints(
    FEProblemBase & from_problem,
    const MooseVariableFieldBase & from_var,
    const MultiAppCoordTransform & from_app_transform,
    std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> & idi)
{
  auto & from_moose_mesh = from_problem.mesh(_displaced_source_mesh);
  const auto & from_mesh = from_moose_mesh.getMesh();

  // Moose system
  const SystemBase & from_system_base = from_var.sys();
  // libmesh system
  const System & from_sys = from_system_base.system();

  // System number and var number
  auto from_sys_num = from_sys.number();
  auto from_var_num = from_sys.variable_number(from_var.name());

  // Check FE type so we can figure out how to sample points
  const auto & fe_type = from_sys.variable_type(from_var_num);
  bool from_is_constant = fe_type.order == CONSTANT;
  bool from_is_nodal = fe_type.family == LAGRANGE;

  // Currently, for an elemental variable, we support the constant and first order
  if (fe_type.order > FIRST && !from_is_nodal)
    mooseError("We don't currently support second order or higher elemental variable ");

  // Containers for points and values
  // We later will push data into these containers
  std::vector<Point> & src_pts(idi->get_source_points());
  std::vector<Number> & src_vals(idi->get_source_vals());

  // How much we want to translate mesh if users ask
  std::unordered_map<dof_id_type, Point> from_tranforms;
  std::set<subdomain_id_type> exclude_block_ids;
  if (_shrink_gap_width > 0 && _shrink_mesh == "source")
  {
    computeTransformation(from_moose_mesh, from_tranforms);
    auto exclude_subdomainids = from_moose_mesh.getSubdomainIDs(_exclude_gap_blocks);
    exclude_block_ids.insert(exclude_subdomainids.begin(), exclude_subdomainids.end());
  }

  // The solution from the system with which the from_var is associated
  const NumericVector<Number> & from_solution = *from_sys.solution;

  std::set<subdomain_id_type> subdomainids;
  std::vector<subdomain_id_type> include_block_ids;
  if (from_is_nodal)
  {
    for (const auto * const from_node : from_mesh.local_node_ptr_range())
    {
      // Assuming LAGRANGE!
      if (from_node->n_comp(from_sys_num, from_var_num) == 0)
        continue;

      Point translate(0);

      if (from_tranforms.size() > 0)
      {
        subdomainIDsNode(const_cast<MooseMesh &>(from_moose_mesh), *from_node, subdomainids);
        // Check if node is excluded
        // Node will be excluded if it is in the interior of excluded subdomains
        include_block_ids.clear();
        include_block_ids.resize(std::max(subdomainids.size(), exclude_block_ids.size()));
        auto it = std::set_difference(subdomainids.begin(),
                                      subdomainids.end(),
                                      exclude_block_ids.begin(),
                                      exclude_block_ids.end(),
                                      include_block_ids.begin());

        include_block_ids.resize(it - include_block_ids.begin());
        // Node is not excluded
        if (include_block_ids.size())
          translate = from_tranforms[*include_block_ids.begin()];
        else
          continue;
      }

      // Push value and point to KDTree
      dof_id_type from_dof = from_node->dof_number(from_sys_num, from_var_num, 0);
      src_vals.push_back(from_solution(from_dof));
      src_pts.push_back(from_app_transform(*from_node) + translate);
    }
  }
  else
  {
    std::vector<Point> points;
    for (const auto * const from_elem :
         as_range(from_mesh.local_elements_begin(), from_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (from_elem->n_dofs(from_sys_num, from_var_num) < 1)
        continue;

      points.clear();
      if (from_is_constant)
        points.push_back(from_elem->vertex_average());
      else
        for (const auto & node : from_elem->node_ref_range())
          points.push_back(node);

      unsigned int n_comp = from_elem->n_comp(from_sys_num, from_var_num);
      auto n_points = points.size();
      // We assume each point corresponds to one component of elemental variable
      if (n_points != n_comp)
        mooseError(" Number of points ",
                   n_points,
                   " does not equal to number of variable components ",
                   n_comp);

      unsigned int offset = 0;

      Point translate(0);

      if (from_tranforms.size() > 0)
      {
        auto subdomain = from_elem->subdomain_id();

        if (subdomain == Moose::INVALID_BLOCK_ID)
          mooseError("subdomain id does not make sense", subdomain);

        // subdomain is not excluded
        if (exclude_block_ids.find(subdomain) == exclude_block_ids.end())
          translate = from_tranforms[subdomain];
        else
          continue;
      }

      for (const auto & point : points)
      {
        dof_id_type from_dof = from_elem->dof_number(from_sys_num, from_var_num, offset++);
        src_vals.push_back(from_solution(from_dof));
        src_pts.push_back(from_app_transform(point) + translate);
      }
    }
  }
}

void
MultiAppGeometricInterpolationTransfer::interpolateTargetPoints(
    FEProblemBase & to_problem,
    MooseVariableFieldBase & to_var,
    NumericVector<Real> & to_solution,
    const MultiAppCoordTransform & to_app_transform,
    const std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> & idi)
{
  // Moose system
  SystemBase & to_system_base = to_var.sys();
  // libmesh system
  System & to_sys = to_system_base.system();

  // System number and var number
  auto to_sys_num = to_sys.number();
  auto to_var_num = to_sys.variable_number(to_var.name());

  const MooseMesh & to_moose_mesh = to_problem.mesh(_displaced_target_mesh);
  const MeshBase & to_mesh = to_moose_mesh.getMesh();

  // Compute transform info
  std::unordered_map<dof_id_type, Point> to_tranforms;
  std::set<subdomain_id_type> exclude_block_ids;
  if (_shrink_gap_width > 0 && _shrink_mesh == "target")
  {
    computeTransformation(to_moose_mesh, to_tranforms);
    auto exclude_subdomainids = to_moose_mesh.getSubdomainIDs(_exclude_gap_blocks);
    exclude_block_ids.insert(exclude_subdomainids.begin(), exclude_subdomainids.end());
  }

  const auto & to_fe_type = to_sys.variable_type(to_var_num);
  bool to_is_constant = to_fe_type.order == CONSTANT;
  bool to_is_nodal = to_fe_type.family == LAGRANGE;

  if (to_fe_type.order > FIRST && !to_is_nodal)
    mooseError("We don't currently support second order or higher elemental variable ");

  std::set<subdomain_id_type> subdomainids;
  std::vector<subdomain_id_type> include_block_ids;
  std::vector<Point> pts;
  std::vector<Number> vals;
  if (to_is_nodal)
  {
    for (const auto * const node : to_mesh.local_node_ptr_range())
    {
      if (node->n_dofs(to_sys_num, to_var_num) <= 0) // If this variable has dofs at this node
        continue;

      Point translate(0);
      if (to_tranforms.size() > 0)
      {
        subdomainIDsNode(const_cast<MooseMesh &>(to_moose_mesh), *node, subdomainids);
        // Check if node is excluded
        // Node will be excluded if it is in the interior of excluded subdomains
        include_block_ids.clear();
        include_block_ids.resize(std::max(subdomainids.size(), exclude_block_ids.size()));
        auto it = std::set_difference(subdomainids.begin(),
                                      subdomainids.end(),
                                      exclude_block_ids.begin(),
                                      exclude_block_ids.end(),
                                      include_block_ids.begin());
        include_block_ids.resize(it - include_block_ids.begin());
        if (include_block_ids.size())
          translate = to_tranforms[*include_block_ids.begin()];
        else
          continue;
      }

      pts.clear();
      pts.push_back(to_app_transform(*node) + translate);
      vals.resize(1);

      idi->interpolate_field_data({_to_var_name}, pts, vals);
      dof_id_type dof = node->dof_number(to_sys_num, to_var_num, 0);
      to_solution.set(dof, vals.front());
    }
  }
  else // Elemental
  {
    std::vector<Point> points;
    for (const auto * const elem :
         as_range(to_mesh.local_elements_begin(), to_mesh.local_elements_end()))
    {
      // Skip this element if the variable has no dofs at it.
      if (elem->n_dofs(to_sys_num, to_var_num) < 1)
        continue;

      points.clear();
      if (to_is_constant)
        points.push_back(elem->vertex_average());
      else
        for (const auto & node : elem->node_ref_range())
          points.push_back(node);

      auto n_points = points.size();
      unsigned int n_comp = elem->n_comp(to_sys_num, to_var_num);
      // We assume each point corresponds to one component of elemental variable
      if (n_points != n_comp)
        mooseError(" Number of points ",
                   n_points,
                   " does not equal to number of variable components ",
                   n_comp);

      Point translate(0);

      if (to_tranforms.size() > 0)
      {
        auto subdomain = elem->subdomain_id();

        if (subdomain == Moose::INVALID_BLOCK_ID)
          mooseError("subdomain id does not make sense", subdomain);

        if (exclude_block_ids.find(subdomain) == exclude_block_ids.end())
          translate = to_tranforms[subdomain];
        else
          continue;
      }

      unsigned int offset = 0;
      for (const auto & point : points)
      {
        pts.clear();
        pts.push_back(to_app_transform(point) + translate);
        vals.resize(1);

        idi->interpolate_field_data({_to_var_name}, pts, vals);
        dof_id_type dof = elem->dof_number(to_sys_num, to_var_num, offset++);
        to_solution.set(dof, vals.front());
      } // point
    }   // auto elem
  }     // else

  to_solution.close();
  to_sys.update();
}

void
MultiAppGeometricInterpolationTransfer::execute()
{
  TIME_SECTION("MultiAppGeometricInterpolationTransfer::execute()",
               5,
               "Transferring variables based on node interpolation");

  const FEProblemBase & fe_problem =
      hasFromMultiApp() ? getFromMultiApp()->problemBase() : getToMultiApp()->problemBase();
  std::unique_ptr<InverseDistanceInterpolation<LIBMESH_DIM>> idi;
  switch (_interp_type)
  {
    case 0:
      idi = std::make_unique<InverseDistanceInterpolation<LIBMESH_DIM>>(
          fe_problem.comm(), _num_points, _power);
      break;
    case 1:
      idi = std::make_unique<RadialBasisInterpolation<LIBMESH_DIM>>(fe_problem.comm(), _radius);
      break;
    default:
      mooseError("Unknown interpolation type!");
  }

  idi->set_field_variables({_to_var_name});

  switch (_current_direction)
  {
    case TO_MULTIAPP:
    {
      FEProblemBase & from_problem = getToMultiApp()->problemBase();
      const auto & from_var = from_problem.getVariable(
          0, _from_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

      mooseAssert(_from_transforms.size() == 1, "This should be size 1");
      fillSourceInterpolationPoints(from_problem, from_var, *_from_transforms[0], idi);

      // We have only set local values - prepare for use by gathering remote gata
      idi->prepare_for_use();

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); i++)
      {
        if (getToMultiApp()->hasLocalApp(i))
        {
          auto & to_problem = getToMultiApp()->appProblemBase(i);
          Moose::ScopedCommSwapper swapper(to_problem.comm().get());
          auto & to_var = to_problem.getVariable(0,
                                                 _to_var_name,
                                                 Moose::VarKindType::VAR_ANY,
                                                 Moose::VarFieldType::VAR_FIELD_STANDARD);

          auto & to_solution = getToMultiApp()->appTransferVector(i, _to_var_name);

          interpolateTargetPoints(to_problem, to_var, to_solution, *_to_transforms[i], idi);
        }
      }

      break;
    }

    case FROM_MULTIAPP:
    {
      for (unsigned int i = 0; i < getFromMultiApp()->numGlobalApps(); i++)
      {
        if (getFromMultiApp()->hasLocalApp(i))
        {
          auto & from_problem = getFromMultiApp()->appProblemBase(i);
          Moose::ScopedCommSwapper swapper(from_problem.comm().get());
          const auto & from_var = from_problem.getVariable(0,
                                                           _from_var_name,
                                                           Moose::VarKindType::VAR_ANY,
                                                           Moose::VarFieldType::VAR_FIELD_STANDARD);

          fillSourceInterpolationPoints(from_problem, from_var, *_from_transforms[i], idi);
        }
      }

      idi->prepare_for_use();

      FEProblemBase & to_problem = getFromMultiApp()->problemBase();
      MooseVariableFieldBase & to_var = to_problem.getVariable(
          0, _to_var_name, Moose::VarKindType::VAR_ANY, Moose::VarFieldType::VAR_FIELD_STANDARD);

      auto & to_solution = *to_var.sys().system().solution;

      mooseAssert(_to_transforms.size() == 1, "This should be size 1");
      interpolateTargetPoints(to_problem, to_var, to_solution, *_to_transforms[0], idi);

      break;
    }
    default:
    {
      mooseError("Unsupported transfer direction  ", _current_direction);
      break;
    }
  }
}

void
MultiAppGeometricInterpolationTransfer::computeTransformation(
    const MooseMesh & mesh, std::unordered_map<dof_id_type, Point> & transformation)
{
  auto & libmesh_mesh = mesh.getMesh();

  auto & subdomainids = mesh.meshSubdomains();

  subdomain_id_type max_subdomain_id = 0;

  // max_subdomain_id will be used to represent the center of the entire domain
  for (auto subdomain_id : subdomainids)
  {
    max_subdomain_id = max_subdomain_id > subdomain_id ? max_subdomain_id : subdomain_id;
  }

  max_subdomain_id += 1;

  std::unordered_map<dof_id_type, Point> subdomain_centers;
  std::unordered_map<dof_id_type, dof_id_type> nelems;

  for (auto & elem :
       as_range(libmesh_mesh.local_elements_begin(), libmesh_mesh.local_elements_end()))
  {
    // Compute center of the entire domain
    subdomain_centers[max_subdomain_id] += elem->vertex_average();
    nelems[max_subdomain_id] += 1;

    auto subdomain = elem->subdomain_id();

    if (subdomain == Moose::INVALID_BLOCK_ID)
      mooseError("block is invalid");

    // Centers for subdomains
    subdomain_centers[subdomain] += elem->vertex_average();

    nelems[subdomain] += 1;
  }

  comm().sum(subdomain_centers);

  comm().sum(nelems);

  subdomain_centers[max_subdomain_id] /= nelems[max_subdomain_id];

  for (auto subdomain_id : subdomainids)
  {
    subdomain_centers[subdomain_id] /= nelems[subdomain_id];
  }

  // Compute unit vectors representing directions in which we want to shrink mesh
  // The unit vectors is scaled by 'shrink_gap_width'
  transformation.clear();
  for (auto subdomain_id : subdomainids)
  {
    transformation[subdomain_id] =
        subdomain_centers[max_subdomain_id] - subdomain_centers[subdomain_id];

    auto norm = transformation[subdomain_id].norm();

    // The current subdomain is the center of the entire domain,
    // then we do not move this subdomain
    if (norm > _distance_tol)
      transformation[subdomain_id] /= norm;

    transformation[subdomain_id] *= _shrink_gap_width;
  }
}

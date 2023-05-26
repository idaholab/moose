//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CrackFrontDefinition.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "RankTwoTensor.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature.h"

registerMooseObject("TensorMechanicsApp", CrackFrontDefinition);

InputParameters
CrackFrontDefinition::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Used to describe geometric characteristics of the crack front for "
                             "fracture integral calculations");
  params += BoundaryRestrictable::validParams();
  addCrackFrontDefinitionParams(params);
  params.set<bool>("use_displaced_mesh") = false;

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 2; });
  return params;
}

void
addCrackFrontDefinitionParams(InputParameters & params)
{
  MooseEnum direction_method("CrackDirectionVector CrackMouth CurvedCrackFront");
  MooseEnum end_direction_method("NoSpecialTreatment CrackDirectionVector CrackTangentVector",
                                 "NoSpecialTreatment");
  params.addParam<std::vector<Point>>("crack_front_points", "Set of points to define crack front");
  params.addParam<bool>("closed_loop", false, "Set of points forms forms a closed loop");
  params.addRequiredParam<MooseEnum>(
      "crack_direction_method",
      direction_method,
      "Method to determine direction of crack propagation.  Choices are: " +
          direction_method.getRawNames());
  params.addParam<MooseEnum>(
      "crack_end_direction_method",
      end_direction_method,
      "Method to determine direction of crack propagation at ends of crack.  Choices are: " +
          end_direction_method.getRawNames());
  params.addParam<RealVectorValue>("crack_direction_vector", "Direction of crack propagation");
  params.addParam<RealVectorValue>(
      "crack_direction_vector_end_1",
      "Direction of crack propagation for the node at end 1 of the crack");
  params.addParam<RealVectorValue>(
      "crack_direction_vector_end_2",
      "Direction of crack propagation for the node at end 2 of the crack");
  params.addParam<RealVectorValue>("crack_tangent_vector_end_1",
                                   "Direction of crack tangent for the node at end 1 of the crack");
  params.addParam<RealVectorValue>("crack_tangent_vector_end_2",
                                   "Direction of crack tangent for the node at end 2 of the crack");
  params.addParam<std::vector<BoundaryName>>(
      "crack_mouth_boundary", "Boundaries whose average coordinate defines the crack mouth");
  params.addParam<std::vector<BoundaryName>>("intersecting_boundary",
                                             "Boundaries intersected by ends of crack");
  params.addParam<bool>("2d", false, "Treat body as two-dimensional");
  params.addRangeCheckedParam<unsigned int>(
      "axis_2d",
      2,
      "axis_2d>=0 & axis_2d<=2",
      "Out of plane axis for models treated as two-dimensional (0=x, 1=y, 2=z)");
  params.addParam<unsigned int>("symmetry_plane",
                                "Account for a symmetry plane passing through "
                                "the plane of the crack, normal to the specified "
                                "axis (0=x, 1=y, 2=z)");
  params.addParam<bool>("t_stress", false, "Calculate T-stress");
  params.addParam<bool>("q_function_rings", false, "Generate rings of nodes for q-function");
  params.addParam<unsigned int>("last_ring", "The number of rings of nodes to generate");
  params.addParam<unsigned int>("first_ring", "The number of rings of nodes to generate");
  params.addParam<unsigned int>("nrings", "The number of rings of nodes to generate");
  params.addParam<VariableName>("disp_x", "Variable containing the x displacement");
  params.addParam<VariableName>("disp_y", "Variable containing the y displacement");
  params.addParam<VariableName>("disp_z", "Variable containing the z displacement");
  params.addParam<std::vector<Real>>("j_integral_radius_inner",
                                     "Radius for J-Integral calculation");
  params.addParam<std::vector<Real>>("j_integral_radius_outer",
                                     "Radius for J-Integral calculation");
  MooseEnum q_function_type("Geometry Topology", "Geometry");
  params.addParam<MooseEnum>("q_function_type",
                             q_function_type,
                             "The method used to define the integration domain. Options are: " +
                                 q_function_type.getRawNames());
  params.addParam<UserObjectName>(
      "crack_front_points_provider",
      "The UserObject provides the crack front points from XFEM GeometricCutObject");

  params.addParam<unsigned int>(
      "number_points_from_provider",
      "The number of crack front points, only needed if crack_front_points_provider is used.");
}

const Real CrackFrontDefinition::_tol = 1e-10;

CrackFrontDefinition::CrackFrontDefinition(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    BoundaryRestrictable(this, true), // false means nodesets
    _direction_method(getParam<MooseEnum>("crack_direction_method").getEnum<DIRECTION_METHOD>()),
    _end_direction_method(
        getParam<MooseEnum>("crack_end_direction_method").getEnum<END_DIRECTION_METHOD>()),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_subproblem.mesh()),
    _treat_as_2d(getParam<bool>("2d")),
    _use_mesh_cutter(false),
    _is_cutter_modified(false),
    _closed_loop(getParam<bool>("closed_loop")),
    _axis_2d(getParam<unsigned int>("axis_2d")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _symmetry_plane(_has_symmetry_plane ? getParam<unsigned int>("symmetry_plane")
                                        : std::numeric_limits<unsigned int>::max()),
    _t_stress(getParam<bool>("t_stress")),
    _q_function_rings(getParam<bool>("q_function_rings")),
    _q_function_type(getParam<MooseEnum>("q_function_type")),
    _crack_front_points_provider(nullptr)
{
  if (isParamValid("crack_front_points"))
  {
    if (isParamValid("boundary"))
      paramError("crack_front_points",
                 "CrackFrontDefinition error: since boundary is defined, crack_front_points should "
                 "not be added.");
    if (isParamValid("crack_front_points_provider"))
      paramError("crack_front_points_provider",
                 "As crack_front_points have been provided, the crack_front_points_provider will "
                 "not be used and needs to be removed.");
    _crack_front_points = getParam<std::vector<Point>>("crack_front_points");
    _geom_definition_method = CRACK_GEOM_DEFINITION::CRACK_FRONT_POINTS;
    if (_t_stress)
      paramError("t_stress", "t_stress not yet supported with crack_front_points");
    if (_q_function_rings)
      paramError("q_function_rings", "q_function_rings not supported with crack_front_points");
  }
  else if (isParamValid("crack_front_points_provider"))
  {
    if (isParamValid("boundary"))
      paramError("crack_front_points_provider",
                 "CrackFrontDefinition error: since boundary is defined, "
                 "crack_front_points_provider should not be added.");
    if (!isParamValid("number_points_from_provider"))
      paramError("number_points_from_provider",
                 "CrackFrontDefinition error: When crack_front_points_provider is used, the "
                 "number_points_from_provider must be provided.");

    _num_points_from_provider = getParam<unsigned int>("number_points_from_provider");
    _geom_definition_method = CRACK_GEOM_DEFINITION::CRACK_FRONT_POINTS;
  }
  else if (isParamValid("number_points_from_provider"))
    paramError("number_points_from_provider",
               "CrackFrontDefinition error: number_points_from_provider is provided but "
               "crack_front_points_provider cannot be found.");
  else if (isParamValid("boundary"))
  {
    _geom_definition_method = CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES;
    if (parameters.isParamSetByUser("closed_loop"))
      paramError("closed_loop",
                 "In CrackFrontDefinition, if 'boundary' is defined, 'closed_loop' should not be "
                 "set by user because closed loops are detected automatically");
  }
  else
    mooseError("In CrackFrontDefinition, must define one of 'boundary', 'crack_front_points' "
               "and 'crack_front_points_provider'");

  if (isParamValid("crack_mouth_boundary"))
    _crack_mouth_boundary_names = getParam<std::vector<BoundaryName>>("crack_mouth_boundary");

  if (_has_symmetry_plane)
    if (_symmetry_plane > 2)
      paramError("symmetry_plane",
                 "symmetry_plane out of bounds: ",
                 _symmetry_plane,
                 " Must be >=0 and <=2.");

  switch (_direction_method)
  {
    case DIRECTION_METHOD::CRACK_DIRECTION_VECTOR:
      if (!isParamValid("crack_direction_vector"))
        paramError("crack_direction_vector",
                   "crack_direction_vector must be specified if crack_direction_method = "
                   "CrackDirectionVector");
      _crack_direction_vector = getParam<RealVectorValue>("crack_direction_vector");
      break;
    case DIRECTION_METHOD::CRACK_MOUTH:
      if (isParamValid("crack_direction_vector"))
        paramError("crack_direction_vector",
                   "crack_direction_vector must not be specified if crack_direction_method = "
                   "CrackMouthNodes");
      if (_crack_mouth_boundary_names.size() == 0)
        paramError(
            "crack_mouth_boundary",
            "crack_mouth_boundary must be specified if crack_direction_method = CrackMouthNodes");
      break;
    case DIRECTION_METHOD::CURVED_CRACK_FRONT:
      if (isParamValid("crack_direction_vector"))
        paramError("crack_direction_vector",
                   "crack_direction_vector must not be specified if crack_direction_method = "
                   "CurvedCrackFront");
      break;
    default:
      paramError("crack_direction_method", "Invalid direction_method");
  }

  if (isParamValid("intersecting_boundary"))
    _intersecting_boundary_names = getParam<std::vector<BoundaryName>>("intersecting_boundary");

  if (_end_direction_method == END_DIRECTION_METHOD::END_CRACK_DIRECTION_VECTOR)
  {
    if (!isParamValid("crack_direction_vector_end_1"))
      paramError("crack_direction_vector_end_1",
                 "crack_direction_vector_end_1 must be specified if crack_end_direction_method = "
                 "CrackDirectionVector");
    if (!isParamValid("crack_direction_vector_end_2"))
      paramError("crack_direction_vector_end_2",
                 "crack_direction_vector_end_2 must be specified if crack_end_direction_method = "
                 "CrackDirectionVector");
    _crack_direction_vector_end_1 = getParam<RealVectorValue>("crack_direction_vector_end_1");
    _crack_direction_vector_end_2 = getParam<RealVectorValue>("crack_direction_vector_end_2");
  }

  if (_end_direction_method == END_DIRECTION_METHOD::END_CRACK_TANGENT_VECTOR)
  {
    if (!isParamValid("crack_tangent_vector_end_1"))
      paramError("crack_tangent_vector_end_1",
                 "crack_tangent_vector_end_1 must be specified if crack_end_tangent_method = "
                 "CrackTangentVector");
    if (!isParamValid("crack_tangent_vector_end_2"))
      paramError("crack_tangent_vector_end_2",
                 "crack_tangent_vector_end_2 must be specified if crack_end_tangent_method = "
                 "CrackTangentVector");
    _crack_tangent_vector_end_1 = getParam<RealVectorValue>("crack_tangent_vector_end_1");
    _crack_tangent_vector_end_2 = getParam<RealVectorValue>("crack_tangent_vector_end_2");
  }

  if (isParamValid("disp_x") && isParamValid("disp_y") && isParamValid("disp_z"))
  {
    _disp_x_var_name = getParam<VariableName>("disp_x");
    _disp_y_var_name = getParam<VariableName>("disp_y");
    _disp_z_var_name = getParam<VariableName>("disp_z");
  }
  else if (_t_stress == true && _treat_as_2d == false)
    paramError("displacements", "Displacement variables must be provided for T-stress calculation");

  if (_q_function_rings)
  {
    if (!isParamValid("last_ring"))
      paramError("last_ring",
                 "The max number of rings of nodes to generate must be provided if "
                 "q_function_rings = true");
    _last_ring = getParam<unsigned int>("last_ring");
    _first_ring = getParam<unsigned int>("first_ring");
  }
  else
  {
    _j_integral_radius_inner = getParam<std::vector<Real>>("j_integral_radius_inner");
    _j_integral_radius_outer = getParam<std::vector<Real>>("j_integral_radius_outer");
  }
}

CrackFrontDefinition::~CrackFrontDefinition() {}

void
CrackFrontDefinition::execute()
{
  // Because J-Integral is based on original geometry, the crack front geometry
  // is never updated, so everything that needs to happen is done in initialSetup()
  // fixme Lynn Help with this Benjamin Spencer.  Not suer if this is true after this commit
  if (_t_stress == true && _treat_as_2d == false)
    calculateTangentialStrainAlongFront();
}

void
CrackFrontDefinition::initialSetup()
{
  if (isParamValid("crack_front_points_provider"))
  {
    _crack_front_points_provider = &getUserObjectByName<CrackFrontPointsProvider>(
        getParam<UserObjectName>("crack_front_points_provider"));
    if (_crack_front_points_provider->usesMesh())
    {
      _use_mesh_cutter = true;
      if (_direction_method != DIRECTION_METHOD::CURVED_CRACK_FRONT)
        paramError("crack_direction_method",
                   "Using a `crack_front_points_provider` that uses an XFEM cutter mesh also "
                   "requires setting 'crack_direction_method = CurvedCrackFront'");
      if (isParamValid("crack_mouth_boundary"))
        paramError("crack_mouth_boundary",
                   "'crack_mouth_boundary' cannot be set when using a "
                   "'crack_front_points_provider' that uses an XFEM cutter mesh");
    }
  }
  if (_crack_front_points_provider != nullptr)
  {
    // TODO: For crack nucleation, should call a new method on the _crack_front_points_provider to
    // get the number of crack front points IF the crack_front_points_provider's initialSetup has
    // been called
    _crack_front_points =
        _crack_front_points_provider->getCrackFrontPoints(_num_points_from_provider);
    if (_use_mesh_cutter)
      _crack_plane_normals =
          _crack_front_points_provider->getCrackPlaneNormals(_num_points_from_provider);
  }

  _crack_mouth_boundary_ids = _mesh.getBoundaryIDs(_crack_mouth_boundary_names, true);
  _intersecting_boundary_ids = _mesh.getBoundaryIDs(_intersecting_boundary_names, true);

  if (_geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES)
  {
    std::set<dof_id_type> nodes;
    getCrackFrontNodes(nodes);
    orderCrackFrontNodes(nodes);
  }

  if (_closed_loop && _intersecting_boundary_names.size() > 0)
    paramError("intersecting_boundary", "Cannot use intersecting_boundary with closed-loop cracks");

  updateCrackFrontGeometry();

  if (_q_function_rings)
    createQFunctionRings();

  if (_t_stress)
  {
    std::size_t num_crack_front_nodes = _ordered_crack_front_nodes.size();
    for (std::size_t i = 0; i < num_crack_front_nodes; ++i)
      _strain_along_front.push_back(-std::numeric_limits<Real>::max());
  }

  std::size_t num_crack_front_points = getNumCrackFrontPoints();
  if (_q_function_type == "GEOMETRY")
  {
    if (!_treat_as_2d)
      if (num_crack_front_points < 1)
        mooseError("num_crack_front_points is not > 0");
    for (std::size_t i = 0; i < num_crack_front_points; ++i)
    {
      bool is_point_on_intersecting_boundary = isPointWithIndexOnIntersectingBoundary(i);
      _is_point_on_intersecting_boundary.push_back(is_point_on_intersecting_boundary);
    }
  }
}

void
CrackFrontDefinition::initialize()
{
  // Update the crack front for fracture integral calculations
  // This is only useful for growing cracks which are currently described by the mesh
  // cutter
  if (_use_mesh_cutter && _is_cutter_modified)
  {
    // TODO: For crack nucleation, should call a new method on the _crack_front_points_provider to
    // get the number of crack front points IF the crack_front_points_provider's initialSetup has
    // been called.  This also needs to be address in line 304
    _crack_front_points =
        _crack_front_points_provider->getCrackFrontPoints(_num_points_from_provider);
    _crack_plane_normals =
        _crack_front_points_provider->getCrackPlaneNormals(_num_points_from_provider);
    updateCrackFrontGeometry();
    std::size_t num_crack_front_points = getNumCrackFrontPoints();
    if (_q_function_type == "GEOMETRY")
      for (std::size_t i = 0; i < num_crack_front_points; ++i)
      {
        bool is_point_on_intersecting_boundary = isPointWithIndexOnIntersectingBoundary(i);
        _is_point_on_intersecting_boundary.push_back(is_point_on_intersecting_boundary);
      }
  }
}

void
CrackFrontDefinition::finalize()
{
  if (_t_stress)
    _communicator.max(_strain_along_front);
}

void
CrackFrontDefinition::getCrackFrontNodes(std::set<dof_id_type> & nodes)
{
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (auto nd = bnd_nodes.begin(); nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;

    if (hasBoundary(boundary_id))
      nodes.insert(bnode->_node->id());
  }

  if (_treat_as_2d && _use_mesh_cutter == false)
  {
    if (nodes.size() > 1)
    {
      // Check that the nodes are collinear in the axis normal to the 2d plane
      unsigned int axis0;
      unsigned int axis1;

      switch (_axis_2d)
      {
        case 0:
          axis0 = 1;
          axis1 = 2;
          break;
        case 1:
          axis0 = 0;
          axis1 = 2;
          break;
        case 2:
          axis0 = 0;
          axis1 = 1;
          break;
        default:
          mooseError("Invalid axis.");
      }

      Real node0coor0 = 0;
      Real node0coor1 = 0;

      for (auto sit = nodes.begin(); sit != nodes.end(); ++sit)
      {
        Node & curr_node = _mesh.nodeRef(*sit);
        if (sit == nodes.begin())
        {
          node0coor0 = curr_node(axis0);
          node0coor1 = curr_node(axis1);
        }
        else
        {
          if (!MooseUtils::absoluteFuzzyEqual(curr_node(axis0), node0coor0, _tol) ||
              !MooseUtils::absoluteFuzzyEqual(curr_node(axis1), node0coor1, _tol))
            mooseError("Boundary provided in CrackFrontDefinition contains ",
                       nodes.size(),
                       " nodes, which are not collinear in the ",
                       _axis_2d,
                       " axis.  Must contain either 1 node or collinear nodes to treat as 2D.");
        }
      }
    }
  }
}

void
CrackFrontDefinition::orderCrackFrontNodes(std::set<dof_id_type> & nodes)
{
  _ordered_crack_front_nodes.clear();
  if (nodes.size() < 1)
    mooseError("No crack front nodes");
  else if (nodes.size() == 1)
  {
    _ordered_crack_front_nodes.push_back(*nodes.begin());
    if (!_treat_as_2d)
      mooseError("Boundary provided in CrackFrontDefinition contains 1 node, but model is not "
                 "treated as 2d");
  }
  else if (_treat_as_2d && _use_mesh_cutter)
  {
    // This is for the 2D case that uses a mesh cutter object so every node is its own crack front
    // and is not connected to other crack front nodes.  Copying the order here makes it the same
    // as that given in the MeshCut2DFractureUserObject
    std::copy(nodes.begin(), nodes.end(), _ordered_crack_front_nodes.begin());
  }
  else
  {
    // Loop through the set of crack front nodes, and create a node to element map for just the
    // crack front nodes
    // The main reason for creating a second map is that we need to do a sort prior to the
    // set_intersection.
    // The original map contains vectors, and we can't sort them, so we create sets in the local
    // map.
    const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map =
        _mesh.nodeToElemMap();
    std::map<dof_id_type, std::set<dof_id_type>> crack_front_node_to_elem_map;

    for (const auto & node_id : nodes)
    {
      const auto & node_to_elem_pair = node_to_elem_map.find(node_id);
      mooseAssert(node_to_elem_pair != node_to_elem_map.end(),
                  "Could not find crack front node " << node_id << " in the node to elem map");

      const std::vector<dof_id_type> & connected_elems = node_to_elem_pair->second;
      for (std::size_t i = 0; i < connected_elems.size(); ++i)
        crack_front_node_to_elem_map[node_id].insert(connected_elems[i]);
    }

    // Determine which nodes are connected to each other via elements, and construct line elements
    // to represent
    // those connections
    std::vector<std::vector<dof_id_type>> line_elems;
    std::map<dof_id_type, std::vector<dof_id_type>> node_to_line_elem_map;

    for (auto cfnemit = crack_front_node_to_elem_map.begin();
         cfnemit != crack_front_node_to_elem_map.end();
         ++cfnemit)
    {
      auto cfnemit2 = cfnemit;
      for (++cfnemit2; cfnemit2 != crack_front_node_to_elem_map.end(); ++cfnemit2)
      {

        std::vector<dof_id_type> common_elements;
        std::set<dof_id_type> & elements_connected_to_node1 = cfnemit->second;
        std::set<dof_id_type> & elements_connected_to_node2 = cfnemit2->second;
        std::set_intersection(elements_connected_to_node1.begin(),
                              elements_connected_to_node1.end(),
                              elements_connected_to_node2.begin(),
                              elements_connected_to_node2.end(),
                              std::inserter(common_elements, common_elements.end()));

        if (common_elements.size() > 0)
        {
          std::vector<dof_id_type> my_line_elem;
          my_line_elem.push_back(cfnemit->first);
          my_line_elem.push_back(cfnemit2->first);
          node_to_line_elem_map[cfnemit->first].push_back(line_elems.size());
          node_to_line_elem_map[cfnemit2->first].push_back(line_elems.size());
          line_elems.push_back(my_line_elem);
        }
      }
    }

    // Find nodes on ends of line (those connected to only one line element)
    std::vector<dof_id_type> end_nodes;
    for (auto nlemit = node_to_line_elem_map.begin(); nlemit != node_to_line_elem_map.end();
         ++nlemit)
    {
      std::size_t num_connected_elems = nlemit->second.size();
      if (num_connected_elems == 1)
        end_nodes.push_back(nlemit->first);
      else if (num_connected_elems != 2)
        mooseError(
            "Node ", nlemit->first, " is connected to >2 line segments in CrackFrontDefinition");
    }

    // For embedded crack with closed loop of crack front nodes, must pick the end nodes
    if (end_nodes.size() == 0) // Crack front is a loop.  Pick nodes to be end nodes.
    {
      pickLoopCrackEndNodes(end_nodes, nodes, node_to_line_elem_map, line_elems);
      _closed_loop = true;
      if (_end_direction_method == END_DIRECTION_METHOD::END_CRACK_DIRECTION_VECTOR ||
          _end_direction_method == END_DIRECTION_METHOD::END_CRACK_TANGENT_VECTOR)
        paramError("end_direction_method",
                   "In CrackFrontDefinition, end_direction_method cannot be CrackDirectionVector "
                   "or CrackTangentVector for a closed-loop crack");
      if (_intersecting_boundary_names.size() > 0)
        paramError("intersecting_boundary",
                   "In CrackFrontDefinition, intersecting_boundary cannot be specified for a "
                   "closed-loop crack");
    }
    else if (end_nodes.size() == 2) // Rearrange the order of the end nodes if needed
      orderEndNodes(end_nodes);
    else
      mooseError("In CrackFrontDefinition wrong number of end nodes.  Number end nodes = ",
                 end_nodes.size());

    // Create an ordered list of the nodes going along the line of the crack front
    _ordered_crack_front_nodes.push_back(end_nodes[0]);

    dof_id_type last_node = end_nodes[0];
    dof_id_type second_last_node = last_node;
    while (last_node != end_nodes[1])
    {
      std::vector<dof_id_type> & curr_node_line_elems = node_to_line_elem_map[last_node];
      bool found_new_node = false;
      for (std::size_t i = 0; i < curr_node_line_elems.size(); ++i)
      {
        std::vector<dof_id_type> curr_line_elem = line_elems[curr_node_line_elems[i]];
        for (std::size_t j = 0; j < curr_line_elem.size(); ++j)
        {
          dof_id_type line_elem_node = curr_line_elem[j];
          if (_closed_loop &&
              (last_node == end_nodes[0] &&
               line_elem_node == end_nodes[1])) // wrong direction around closed loop
            continue;
          if (line_elem_node != last_node && line_elem_node != second_last_node)
          {
            _ordered_crack_front_nodes.push_back(line_elem_node);
            found_new_node = true;
            break;
          }
        }
        if (found_new_node)
          break;
      }
      second_last_node = last_node;
      last_node = _ordered_crack_front_nodes.back();
    }
  }
}

void
CrackFrontDefinition::orderEndNodes(std::vector<dof_id_type> & end_nodes)
{
  // Choose the node to be the first node.  Do that based on undeformed coordinates for
  // repeatability.
  Node & node0 = _mesh.nodeRef(end_nodes[0]);
  Node & node1 = _mesh.nodeRef(end_nodes[1]);

  std::size_t num_positive_coor0 = 0;
  std::size_t num_positive_coor1 = 0;
  Real dist_from_origin0 = 0.0;
  Real dist_from_origin1 = 0.0;
  for (std::size_t i = 0; i < 3; ++i)
  {
    dist_from_origin0 += node0(i) * node0(i);
    dist_from_origin1 += node1(i) * node1(i);
    if (MooseUtils::absoluteFuzzyGreaterThan(node0(i), 0.0, _tol))
      ++num_positive_coor0;
    if (MooseUtils::absoluteFuzzyGreaterThan(node1(i), 0.0, _tol))
      ++num_positive_coor1;
  }
  dist_from_origin0 = std::sqrt(dist_from_origin0);
  dist_from_origin1 = std::sqrt(dist_from_origin1);

  bool switch_ends = false;
  if (num_positive_coor1 > num_positive_coor0)
  {
    switch_ends = true;
  }
  else
  {
    if (!MooseUtils::absoluteFuzzyEqual(dist_from_origin1, dist_from_origin0, _tol))
    {
      if (dist_from_origin1 < dist_from_origin0)
        switch_ends = true;
    }
    else
    {
      if (end_nodes[1] < end_nodes[0])
        switch_ends = true;
    }
  }
  if (switch_ends)
  {
    std::size_t tmp_node = end_nodes[1];
    end_nodes[1] = end_nodes[0];
    end_nodes[0] = tmp_node;
  }
}

void
CrackFrontDefinition::pickLoopCrackEndNodes(
    std::vector<dof_id_type> & end_nodes,
    std::set<dof_id_type> & nodes,
    std::map<dof_id_type, std::vector<dof_id_type>> & node_to_line_elem_map,
    std::vector<std::vector<dof_id_type>> & line_elems)
{
  dof_id_type max_dist_node = 0;
  Real min_dist = std::numeric_limits<Real>::max();
  Real max_dist = -std::numeric_limits<Real>::max();
  // Pick the node farthest from the origin as the end node, or the one with
  // the greatest x coordinate if the nodes are equidistant from the origin
  for (auto nit = nodes.begin(); nit != nodes.end(); ++nit)
  {
    Node & node = _mesh.nodeRef(*nit);
    Real dist = node.norm();
    if (dist > max_dist)
    {
      max_dist = dist;
      max_dist_node = *nit;
    }
    else if (dist < min_dist)
      min_dist = dist;
  }

  dof_id_type end_node;
  if (MooseUtils::absoluteFuzzyGreaterThan(max_dist, min_dist, _tol))
    end_node = max_dist_node;
  else
  {
    std::vector<Node *> node_vec;
    for (auto nit = nodes.begin(); nit != nodes.end(); ++nit)
      node_vec.push_back(_mesh.nodePtr(*nit));
    end_node = maxNodeCoor(node_vec);
  }

  end_nodes.push_back(end_node);

  // Find the two nodes connected to the node identified as the end node, and pick one of those to
  // be the other end node
  auto end_node_line_elems = node_to_line_elem_map[end_node];
  if (end_node_line_elems.size() != 2)
    mooseError(
        "Crack front nodes are in a loop, but crack end node is only connected to one other node");
  std::vector<Node *> candidate_other_end_nodes;

  for (std::size_t i = 0; i < 2; ++i)
  {
    auto end_line_elem = line_elems[end_node_line_elems[i]];
    for (std::size_t j = 0; j < end_line_elem.size(); ++j)
    {
      auto line_elem_node = end_line_elem[j];
      if (line_elem_node != end_node)
        candidate_other_end_nodes.push_back(_mesh.nodePtr(line_elem_node));
    }
  }
  if (candidate_other_end_nodes.size() != 2)
    mooseError(
        "Crack front nodes are in a loop, but crack end node is not connected to two other nodes");
  end_nodes.push_back(maxNodeCoor(candidate_other_end_nodes, 1));
}

dof_id_type
CrackFrontDefinition::maxNodeCoor(std::vector<Node *> & nodes, unsigned int dir0)
{
  Real dirs[3];
  if (dir0 == 0)
  {
    dirs[0] = 0;
    dirs[1] = 1;
    dirs[2] = 2;
  }
  else if (dir0 == 1)
  {
    dirs[0] = 1;
    dirs[1] = 2;
    dirs[2] = 0;
  }
  else if (dir0 == 2)
  {
    dirs[0] = 2;
    dirs[1] = 0;
    dirs[2] = 1;
  }
  else
    mooseError("Invalid dir0 in CrackFrontDefinition::maxNodeCoor()");

  Real max_coor0 = -std::numeric_limits<Real>::max();
  std::vector<Node *> max_coor0_nodes;
  for (std::size_t i = 0; i < nodes.size(); ++i)
  {
    Real coor0 = (*nodes[i])(dirs[0]);
    if (coor0 > max_coor0)
      max_coor0 = coor0;
  }
  for (std::size_t i = 0; i < nodes.size(); ++i)
  {
    Real coor0 = (*nodes[i])(dirs[0]);
    if (MooseUtils::absoluteFuzzyEqual(coor0, max_coor0, _tol))
      max_coor0_nodes.push_back(nodes[i]);
  }
  if (max_coor0_nodes.size() > 1)
  {
    Real max_coor1 = -std::numeric_limits<Real>::max();
    std::vector<Node *> max_coor1_nodes;
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
      Real coor1 = (*nodes[i])(dirs[1]);
      if (coor1 > max_coor1)
        max_coor1 = coor1;
    }
    for (std::size_t i = 0; i < nodes.size(); ++i)
    {
      Real coor1 = (*nodes[i])(dirs[1]);
      if (MooseUtils::absoluteFuzzyEqual(coor1, max_coor1, _tol))
        max_coor1_nodes.push_back(nodes[i]);
    }
    if (max_coor1_nodes.size() > 1)
    {
      Real max_coor2 = -std::numeric_limits<Real>::max();
      std::vector<Node *> max_coor2_nodes;
      for (std::size_t i = 0; i < nodes.size(); ++i)
      {
        Real coor2 = (*nodes[i])(dirs[2]);
        if (coor2 > max_coor2)
          max_coor2 = coor2;
      }
      for (std::size_t i = 0; i < nodes.size(); ++i)
      {
        Real coor2 = (*nodes[i])(dirs[2]);
        if (MooseUtils::absoluteFuzzyEqual(coor2, max_coor2, _tol))
          max_coor2_nodes.push_back(nodes[i]);
      }
      if (max_coor2_nodes.size() > 1)
        mooseError("Multiple nodes with same x,y,z coordinates within tolerance");
      else
        return max_coor2_nodes[0]->id();
    }
    else
      return max_coor1_nodes[0]->id();
  }
  else
    return max_coor0_nodes[0]->id();
}

void
CrackFrontDefinition::updateCrackFrontGeometry()
{
  updateDataForCrackDirection();

  _segment_lengths.clear();
  _tangent_directions.clear();
  _crack_directions.clear();
  _overall_length = 0.0;
  _rot_matrix.clear();
  _distances_along_front.clear();
  _angles_along_front.clear();
  _strain_along_front.clear();
  _crack_plane_normals.clear();

  if (_treat_as_2d)
  {
    std::size_t num_crack_front_points = getNumCrackFrontPoints();
    _segment_lengths.reserve(num_crack_front_points);
    _tangent_directions.reserve(num_crack_front_points);
    _crack_directions.reserve(num_crack_front_points);

    for (std::size_t i = 0; i < getNumCrackFrontPoints(); ++i)
    {
      RealVectorValue tangent_direction;
      RealVectorValue crack_direction;
      tangent_direction(_axis_2d) = 1.0;
      _tangent_directions.push_back(tangent_direction);
      const Point * crack_front_point = getCrackFrontPoint(i);
      crack_direction =
          calculateCrackFrontDirection(*crack_front_point, tangent_direction, MIDDLE_NODE, i);
      _crack_directions.push_back(crack_direction);

      RankTwoTensor rot_mat;
      rot_mat.fillRow(0, crack_direction);
      rot_mat(2, _axis_2d) = 1.0;
      if (_use_mesh_cutter)
      {
        _crack_plane_normals =
            _crack_front_points_provider->getCrackPlaneNormals(num_crack_front_points);
        mooseAssert(!_crack_plane_normals.empty(), "_crack_plane_normals is empty.");
        rot_mat.fillRow(1, _crack_plane_normals[i]);
      }
      else
      {
        _crack_plane_normal = tangent_direction.cross(crack_direction);
        rot_mat.fillRow(1, _crack_plane_normal);
      }
      _rot_matrix.push_back(rot_mat);

      _segment_lengths.push_back(std::make_pair(0.0, 0.0));
      _distances_along_front.push_back(0.0);
      _angles_along_front.push_back(0.0);
    }
  }
  else
  {
    std::size_t num_crack_front_points = getNumCrackFrontPoints();
    _segment_lengths.reserve(num_crack_front_points);
    _tangent_directions.reserve(num_crack_front_points);
    _crack_directions.reserve(num_crack_front_points);

    RealVectorValue back_segment;
    Real back_segment_len = 0.0;
    if (_closed_loop)
    {
      back_segment = *getCrackFrontPoint(0) - *getCrackFrontPoint(num_crack_front_points - 1);
      back_segment_len = back_segment.norm();
    }

    for (std::size_t i = 0; i < num_crack_front_points; ++i)
    {
      CRACK_NODE_TYPE ntype;
      if (_closed_loop)
        ntype = MIDDLE_NODE;
      else if (i == 0)
        ntype = END_1_NODE;
      else if (i == num_crack_front_points - 1)
        ntype = END_2_NODE;
      else
        ntype = MIDDLE_NODE;

      RealVectorValue forward_segment;
      Real forward_segment_len;
      if (ntype == END_2_NODE)
        forward_segment_len = 0.0;
      else if (_closed_loop && i == num_crack_front_points - 1)
      {
        forward_segment = *getCrackFrontPoint(0) - *getCrackFrontPoint(i);
        forward_segment_len = forward_segment.norm();
      }
      else
      {
        forward_segment = *getCrackFrontPoint(i + 1) - *getCrackFrontPoint(i);
        forward_segment_len = forward_segment.norm();
        _overall_length += forward_segment_len;
      }

      _segment_lengths.push_back(std::make_pair(back_segment_len, forward_segment_len));
      if (i == 0)
        _distances_along_front.push_back(0.0);
      else
        _distances_along_front.push_back(back_segment_len + _distances_along_front[i - 1]);

      RealVectorValue tangent_direction = back_segment + forward_segment;
      tangent_direction = tangent_direction / tangent_direction.norm();

      // If end tangent directions are given, correct the tangent at the end nodes
      if (_direction_method == DIRECTION_METHOD::CURVED_CRACK_FRONT &&
          _end_direction_method == END_DIRECTION_METHOD::END_CRACK_TANGENT_VECTOR)
      {
        if (ntype == END_1_NODE)
          tangent_direction = _crack_tangent_vector_end_1;
        else if (ntype == END_2_NODE)
          tangent_direction = _crack_tangent_vector_end_2;
      }

      _tangent_directions.push_back(tangent_direction);
      _crack_directions.push_back(
          calculateCrackFrontDirection(*getCrackFrontPoint(i), tangent_direction, ntype, i));

      // correct tangent direction in the case of _use_mesh_cutter
      if (_use_mesh_cutter)
        _tangent_directions[i] = _crack_plane_normals[i].cross(_crack_directions[i]);

      // If the end directions are given by the user, correct also the tangent at the end nodes
      if (_direction_method == DIRECTION_METHOD::CURVED_CRACK_FRONT &&
          _end_direction_method == END_DIRECTION_METHOD::END_CRACK_DIRECTION_VECTOR &&
          (ntype == END_1_NODE || ntype == END_2_NODE))
      {
        if (_use_mesh_cutter)
          _tangent_directions[i] = _crack_plane_normals[i].cross(_crack_directions[i]);
        else
          _tangent_directions[i] = _crack_plane_normal.cross(_crack_directions[i]);
      }

      back_segment = forward_segment;
      back_segment_len = forward_segment_len;
    }

    // For CURVED_CRACK_FRONT, _crack_plane_normal gets computed in updateDataForCrackDirection
    if (_direction_method != DIRECTION_METHOD::CURVED_CRACK_FRONT)
    {
      std::size_t mid_id = (num_crack_front_points - 1) / 2;
      _crack_plane_normal = _tangent_directions[mid_id].cross(_crack_directions[mid_id]);

      // Make sure the normal vector is non-zero
      RealVectorValue zero_vec(0.0);
      if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
        mooseError("Crack plane normal vector evaluates to zero");
    }

    // Calculate angles of each point along the crack front for an elliptical crack projected
    // to a circle.
    if (hasAngleAlongFront())
    {
      RealVectorValue origin_to_first_node = *getCrackFrontPoint(0) - _crack_mouth_coordinates;
      Real hyp = origin_to_first_node.norm();
      RealVectorValue norm_origin_to_first_node = origin_to_first_node / hyp;
      RealVectorValue tangent_to_first_node = -norm_origin_to_first_node.cross(_crack_plane_normal);
      tangent_to_first_node /= tangent_to_first_node.norm();

      for (std::size_t i = 0; i < num_crack_front_points; ++i)
      {
        RealVectorValue origin_to_curr_node = *getCrackFrontPoint(i) - _crack_mouth_coordinates;

        Real adj = origin_to_curr_node * norm_origin_to_first_node;
        Real opp = origin_to_curr_node * tangent_to_first_node;

        Real angle = acos(adj / hyp) * 180.0 / libMesh::pi;
        if (opp < 0.0)
          angle = 360.0 - angle;
        _angles_along_front.push_back(angle);
      }

      // Correct angle on end nodes if they are 0 or 360 to be consistent with neighboring node
      if (num_crack_front_points > 1)
      {
        if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[0], 0.0, _tol) &&
            _angles_along_front[1] > 180.0)
          _angles_along_front[0] = 360.0;
        else if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[0], 360.0, _tol) &&
                 _angles_along_front[1] < 180.0)
          _angles_along_front[0] = 0.0;

        if (MooseUtils::absoluteFuzzyEqual(
                _angles_along_front[num_crack_front_points - 1], 0.0, _tol) &&
            _angles_along_front[num_crack_front_points - 2] > 180.0)
          _angles_along_front[num_crack_front_points - 1] = 360.0;
        else if (MooseUtils::absoluteFuzzyEqual(
                     _angles_along_front[num_crack_front_points - 1], 360.0, _tol) &&
                 _angles_along_front[num_crack_front_points - 2] < 180.0)
          _angles_along_front[num_crack_front_points - 1] = 0.0;
      }
    }
    else
      _angles_along_front.resize(num_crack_front_points, 0.0);

    // Create rotation matrix
    for (std::size_t i = 0; i < num_crack_front_points; ++i)
    {
      RankTwoTensor rot_mat;
      rot_mat.fillRow(0, _crack_directions[i]);
      if (_use_mesh_cutter)
        rot_mat.fillRow(1, _crack_plane_normals[i]);
      else
        rot_mat.fillRow(1, _crack_plane_normal);
      rot_mat.fillRow(2, _tangent_directions[i]);
      _rot_matrix.push_back(rot_mat);
    }

    _console << "Summary of crack front geometry (used for fracture integrals):" << std::endl;
    _console << "index   node id   x coord       y coord       z coord       x dir         y dir   "
                "       z dir        angle        position     seg length"
             << std::endl;
    for (std::size_t i = 0; i < num_crack_front_points; ++i)
    {
      std::size_t point_id;
      if (_geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES)
        point_id = _ordered_crack_front_nodes[i];
      else
        point_id = i;
      _console << std::left << std::setw(8) << i + 1 << std::setw(10) << point_id << std::setw(14)
               << (*getCrackFrontPoint(i))(0) << std::setw(14) << (*getCrackFrontPoint(i))(1)
               << std::setw(14) << (*getCrackFrontPoint(i))(2) << std::setw(14)
               << _crack_directions[i](0) << std::setw(14) << _crack_directions[i](1)
               << std::setw(14) << _crack_directions[i](2);
      if (hasAngleAlongFront())
        _console << std::left << std::setw(14) << _angles_along_front[i];
      else
        _console << std::left << std::setw(14) << "--";
      _console << std::left << std::setw(14) << _distances_along_front[i] << std::setw(14)
               << (_segment_lengths[i].first + _segment_lengths[i].second) / 2.0 << std::endl;
    }
    _console << "overall length: " << _overall_length << std::endl;
  }
}

void
CrackFrontDefinition::updateDataForCrackDirection()
{
  if (_crack_mouth_boundary_ids.size() > 0)
  {
    _crack_mouth_coordinates.zero();

    std::set<Node *> crack_mouth_nodes;
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (auto nd = bnd_nodes.begin(); nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;

      for (std::size_t ibid = 0; ibid < _crack_mouth_boundary_ids.size(); ++ibid)
      {
        if (boundary_id == _crack_mouth_boundary_ids[ibid])
        {
          crack_mouth_nodes.insert(bnode->_node);
          break;
        }
      }
    }

    for (auto nit = crack_mouth_nodes.begin(); nit != crack_mouth_nodes.end(); ++nit)
    {
      _crack_mouth_coordinates += **nit;
    }
    _crack_mouth_coordinates /= static_cast<Real>(crack_mouth_nodes.size());

    if (_has_symmetry_plane)
      _crack_mouth_coordinates(_symmetry_plane) = 0.0;
  }

  if (_direction_method == DIRECTION_METHOD::CURVED_CRACK_FRONT && !_use_mesh_cutter)
  {
    _crack_plane_normal.zero();

    // Get 3 nodes on crack front
    std::size_t num_points = getNumCrackFrontPoints();
    if (num_points < 3)
    {
      mooseError("Crack front must contain at least 3 nodes to use CurvedCrackFront option");
    }
    std::size_t start_id;
    std::size_t mid_id;
    std::size_t end_id;

    if (_closed_loop)
    {
      start_id = 0;
      mid_id = (num_points - 1) / 3;
      end_id = 2 * mid_id;
    }
    else
    {
      start_id = 0;
      mid_id = (num_points - 1) / 2;
      end_id = num_points - 1;
    }
    const Point * start = getCrackFrontPoint(start_id);
    const Point * mid = getCrackFrontPoint(mid_id);
    const Point * end = getCrackFrontPoint(end_id);

    // Create two vectors connecting them
    RealVectorValue v1 = *mid - *start;
    RealVectorValue v2 = *end - *mid;

    // Take cross product to get normal
    _crack_plane_normal = v1.cross(v2);
    _crack_plane_normal = _crack_plane_normal.unit();

    // Make sure they're not collinear
    RealVectorValue zero_vec(0.0);
    if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
    {
      mooseError("Nodes on crack front are too close to being collinear");
    }
  }
}

RealVectorValue
CrackFrontDefinition::calculateCrackFrontDirection(const Point & crack_front_point,
                                                   const RealVectorValue & tangent_direction,
                                                   const CRACK_NODE_TYPE ntype,
                                                   const std::size_t crack_front_point_index) const
{
  RealVectorValue crack_dir;
  RealVectorValue zero_vec(0.0);

  bool calc_dir = true;
  if (_end_direction_method == END_DIRECTION_METHOD::END_CRACK_DIRECTION_VECTOR)
  {
    if (ntype == END_1_NODE)
    {
      crack_dir = _crack_direction_vector_end_1;
      calc_dir = false;
    }
    else if (ntype == END_2_NODE)
    {
      crack_dir = _crack_direction_vector_end_2;
      calc_dir = false;
    }
  }

  if (calc_dir)
  {
    if (_direction_method == DIRECTION_METHOD::CRACK_DIRECTION_VECTOR)
    {
      crack_dir = _crack_direction_vector;
    }
    else if (_direction_method == DIRECTION_METHOD::CRACK_MOUTH)
    {
      if (_crack_mouth_coordinates.absolute_fuzzy_equals(crack_front_point, _tol))
      {
        mooseError("Crack mouth too close to crack front node");
      }
      RealVectorValue mouth_to_front = crack_front_point - _crack_mouth_coordinates;

      RealVectorValue crack_plane_normal = mouth_to_front.cross(tangent_direction);
      if (crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
      {
        mooseError(
            "Vector from crack mouth to crack front node is collinear with crack front segment");
      }

      crack_dir = tangent_direction.cross(crack_plane_normal);
      Real dotprod = crack_dir * mouth_to_front;
      if (dotprod < 0)
      {
        crack_dir = -crack_dir;
      }
    }
    else if (_direction_method == DIRECTION_METHOD::CURVED_CRACK_FRONT)
    {
      if (_use_mesh_cutter)
        crack_dir = tangent_direction.cross(_crack_plane_normals[crack_front_point_index]);
      else
        crack_dir = tangent_direction.cross(_crack_plane_normal);
    }
  }
  crack_dir = crack_dir.unit();

  return crack_dir;
}

void
CrackFrontDefinition::updateNumberOfCrackFrontPoints(std::size_t num_points)
{
  _num_points_from_provider = num_points;
}

const Node *
CrackFrontDefinition::getCrackFrontNodePtr(const std::size_t node_index) const
{
  mooseAssert(node_index < _ordered_crack_front_nodes.size(), "node_index out of range");
  const Node * crack_front_node = _mesh.nodePtr(_ordered_crack_front_nodes[node_index]);
  mooseAssert(crack_front_node != nullptr, "invalid crack front node");
  return crack_front_node;
}

const Point *
CrackFrontDefinition::getCrackFrontPoint(const std::size_t point_index) const
{
  if (_geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES)
  {
    return getCrackFrontNodePtr(point_index);
  }
  else
  {
    mooseAssert(point_index < _crack_front_points.size(), "point_index out of range");
    return &_crack_front_points[point_index];
  }
}

const RealVectorValue &
CrackFrontDefinition::getCrackFrontTangent(const std::size_t point_index) const
{
  mooseAssert(point_index < _tangent_directions.size(), "point_index out of range");
  return _tangent_directions[point_index];
}

Real
CrackFrontDefinition::getCrackFrontForwardSegmentLength(const std::size_t point_index) const
{
  return _segment_lengths[point_index].second;
}

Real
CrackFrontDefinition::getCrackFrontBackwardSegmentLength(const std::size_t point_index) const
{
  return _segment_lengths[point_index].first;
}

const RealVectorValue &
CrackFrontDefinition::getCrackDirection(const std::size_t point_index) const
{
  return _crack_directions[point_index];
}

Real
CrackFrontDefinition::getDistanceAlongFront(const std::size_t point_index) const
{
  return _distances_along_front[point_index];
}

bool
CrackFrontDefinition::hasAngleAlongFront() const
{
  return (_crack_mouth_boundary_names.size() > 0);
}

Real
CrackFrontDefinition::getAngleAlongFront(const std::size_t point_index) const
{
  if (!hasAngleAlongFront())
    paramError(
        "crack_mouth_boundary",
        "In CrackFrontDefinition, Requested angle along crack front, but definition of crack mouth "
        "boundary using 'crack_mouth_boundary' parameter is necessary to do that.");
  return _angles_along_front[point_index];
}

std::size_t
CrackFrontDefinition::getNumCrackFrontPoints() const
{
  if (_geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES)
    return _ordered_crack_front_nodes.size();
  else
    return _crack_front_points.size();
}

RealVectorValue
CrackFrontDefinition::rotateToCrackFrontCoords(const RealVectorValue vector,
                                               const std::size_t point_index) const
{
  return _rot_matrix[point_index] * vector;
}

RealVectorValue
CrackFrontDefinition::rotateFromCrackFrontCoordsToGlobal(const RealVectorValue vector,
                                                         const std::size_t point_index) const
{
  RealVectorValue vec = _rot_matrix[point_index].transpose() * vector;
  return vec;
}

RankTwoTensor
CrackFrontDefinition::rotateToCrackFrontCoords(const RankTwoTensor tensor,
                                               const std::size_t point_index) const
{
  RankTwoTensor tmp_tensor(tensor);
  tmp_tensor.rotate(_rot_matrix[point_index]);
  return tmp_tensor;
}

void
CrackFrontDefinition::calculateRThetaToCrackFront(const Point qp,
                                                  const std::size_t point_index,
                                                  Real & r,
                                                  Real & theta) const
{
  std::size_t num_points = getNumCrackFrontPoints();
  Point closest_point(0.0);
  RealVectorValue closest_point_to_p;

  const Point * crack_front_point = getCrackFrontPoint(point_index);
  RealVectorValue crack_front_point_rot = rotateToCrackFrontCoords(*crack_front_point, point_index);

  RealVectorValue crack_front_edge =
      rotateToCrackFrontCoords(_tangent_directions[point_index], point_index);

  Point p_rot = rotateToCrackFrontCoords(qp, point_index);
  p_rot = p_rot - crack_front_point_rot;

  if (_treat_as_2d)
  {
    // In 2D, the closest node is the crack tip node and the position of the crack tip node is
    // (0,0,0) in the crack front coordinate system
    // In case this is a 3D mesh treated as 2D, project point onto same plane as crack front node.
    // Note: In the crack front coordinate system, z is always in the tangent direction to the crack
    // front
    p_rot(2) = closest_point(2);
    closest_point_to_p = p_rot;

    // Find r, the distance between the qp and the crack front
    RealVectorValue r_vec = p_rot;
    r = r_vec.norm();
  }
  else
  {
    // Loop over crack front points to find the one closest to the point qp
    Real min_dist = std::numeric_limits<Real>::max();
    for (std::size_t pit = 0; pit != num_points; ++pit)
    {
      const Point * crack_front_point = getCrackFrontPoint(pit);
      RealVectorValue crack_point_to_current_point = qp - *crack_front_point;
      Real dist = crack_point_to_current_point.norm();

      if (dist < min_dist)
      {
        min_dist = dist;
        closest_point = *crack_front_point;
      }
    }

    // Rotate coordinates to crack front coordinate system
    closest_point = rotateToCrackFrontCoords(closest_point, point_index);
    closest_point = closest_point - crack_front_point_rot;

    // Find r, the distance between the qp and the crack front
    Real edge_length_sq = crack_front_edge.norm_sq();
    closest_point_to_p = p_rot - closest_point;
    Real perp = crack_front_edge * closest_point_to_p;
    Real dist_along_edge = perp / edge_length_sq;
    RealVectorValue point_on_edge = closest_point + crack_front_edge * dist_along_edge;
    RealVectorValue r_vec = p_rot - point_on_edge;
    r = r_vec.norm();
  }

  // Find theta, the angle between r and the crack front plane
  RealVectorValue crack_plane_normal;
  if (_use_mesh_cutter)
    crack_plane_normal = rotateToCrackFrontCoords(_crack_plane_normals[point_index], point_index);
  else
    crack_plane_normal = rotateToCrackFrontCoords(_crack_plane_normal, point_index);
  Real p_to_plane_dist = std::abs(closest_point_to_p * crack_plane_normal);

  // Determine if qp is above or below the crack plane
  Real y_local = p_rot(1) - closest_point(1);

  // Determine if qp is in front of or behind the crack front
  RealVectorValue p2(p_rot);
  p2(1) = 0;
  RealVectorValue p2_vec = p2 - closest_point;
  Real ahead = crack_front_edge(2) * p2_vec(0) - crack_front_edge(0) * p2_vec(2);

  Real x_local(0);
  if (ahead >= 0)
    x_local = 1;
  else
    x_local = -1;

  // Calculate theta based on in which quadrant in the crack front coordinate
  // system the qp is located
  if (r > 0)
  {
    Real theta_quadrant1(0.0);
    if (MooseUtils::absoluteFuzzyEqual(r, p_to_plane_dist, _tol))
      theta_quadrant1 = 0.5 * libMesh::pi;
    else if (p_to_plane_dist > r)
      mooseError(
          "Invalid distance p_to_plane_dist in CrackFrontDefinition::calculateRThetaToCrackFront");
    else
      theta_quadrant1 = std::asin(p_to_plane_dist / r);

    if (x_local >= 0 && y_local >= 0)
      theta = theta_quadrant1;

    else if (x_local < 0 && y_local >= 0)
      theta = libMesh::pi - theta_quadrant1;

    else if (x_local < 0 && y_local < 0)
      theta = -(libMesh::pi - theta_quadrant1);

    else if (x_local >= 0 && y_local < 0)
      theta = -theta_quadrant1;
  }
  else if (r == 0)
    theta = 0;
  else
    mooseError("Invalid distance r in CrackFrontDefinition::calculateRThetaToCrackFront");
}

std::size_t
CrackFrontDefinition::calculateRThetaToCrackFront(const Point qp, Real & r, Real & theta) const
{
  std::size_t num_points = getNumCrackFrontPoints();

  // Loop over crack front points to find the one closest to the point qp
  Real min_dist = std::numeric_limits<Real>::max();
  std::size_t point_index = 0;
  for (std::size_t pit = 0; pit != num_points; ++pit)
  {
    const Point * crack_front_point = getCrackFrontPoint(pit);
    RealVectorValue crack_point_to_current_point = qp - *crack_front_point;
    Real dist = crack_point_to_current_point.norm();

    if (dist < min_dist)
    {
      min_dist = dist;
      point_index = pit;
    }
  }

  calculateRThetaToCrackFront(qp, point_index, r, theta);

  return point_index;
}

bool
CrackFrontDefinition::isNodeOnIntersectingBoundary(const Node * const node) const
{
  bool is_on_boundary = false;
  mooseAssert(node, "Invalid node");
  dof_id_type node_id = node->id();
  for (std::size_t i = 0; i < _intersecting_boundary_ids.size(); ++i)
  {
    if (_mesh.isBoundaryNode(node_id, _intersecting_boundary_ids[i]))
    {
      is_on_boundary = true;
      break;
    }
  }
  return is_on_boundary;
}

bool
CrackFrontDefinition::isPointWithIndexOnIntersectingBoundary(const std::size_t point_index) const
{
  bool is_on_boundary = false;
  if (_geom_definition_method == CRACK_GEOM_DEFINITION::CRACK_FRONT_NODES)
  {
    const Node * crack_front_node = getCrackFrontNodePtr(point_index);
    is_on_boundary = isNodeOnIntersectingBoundary(crack_front_node);
  }
  else
  {
    // If the intersecting boundary option is used with crack front points, the
    // first and last points are assumed to be on the intersecting boundaries.
    std::size_t num_crack_front_points = getNumCrackFrontPoints();
    if (point_index == 0 || point_index == num_crack_front_points - 1)
      is_on_boundary = true;
  }
  return is_on_boundary;
}

void
CrackFrontDefinition::calculateTangentialStrainAlongFront()
{
  RealVectorValue disp_current_node;
  RealVectorValue disp_previous_node;
  RealVectorValue disp_next_node;

  RealVectorValue forward_segment0;
  RealVectorValue forward_segment1;
  Real forward_segment0_len;
  Real forward_segment1_len;
  RealVectorValue back_segment0;
  RealVectorValue back_segment1;
  Real back_segment0_len;
  Real back_segment1_len;

  std::size_t num_crack_front_nodes = _ordered_crack_front_nodes.size();
  const Node * current_node;
  const Node * previous_node;
  const Node * next_node;

  _strain_along_front.reserve(num_crack_front_nodes);

  // In finalize(), gatherMax builds and distributes the complete strain vector on all processors
  // -> reset the vector every time
  for (std::size_t i = 0; i < num_crack_front_nodes; ++i)
    _strain_along_front[i] = -std::numeric_limits<Real>::max();

  MooseVariable & disp_x_var = _subproblem.getStandardVariable(_tid, _disp_x_var_name);
  MooseVariable & disp_y_var = _subproblem.getStandardVariable(_tid, _disp_y_var_name);
  MooseVariable & disp_z_var = _subproblem.getStandardVariable(_tid, _disp_z_var_name);

  current_node = getCrackFrontNodePtr(0);
  if (current_node->processor_id() == processor_id())
  {
    disp_current_node(0) = disp_x_var.getNodalValue(*current_node);
    disp_current_node(1) = disp_y_var.getNodalValue(*current_node);
    disp_current_node(2) = disp_z_var.getNodalValue(*current_node);

    next_node = getCrackFrontNodePtr(1);
    disp_next_node(0) = disp_x_var.getNodalValue(*next_node);
    disp_next_node(1) = disp_y_var.getNodalValue(*next_node);
    disp_next_node(2) = disp_z_var.getNodalValue(*next_node);

    forward_segment0 = *next_node - *current_node;
    forward_segment0 = (forward_segment0 * _tangent_directions[0]) * _tangent_directions[0];
    forward_segment0_len = forward_segment0.norm();

    forward_segment1 = (*next_node + disp_next_node) - (*current_node + disp_current_node);
    forward_segment1 = (forward_segment1 * _tangent_directions[0]) * _tangent_directions[0];
    forward_segment1_len = forward_segment1.norm();

    _strain_along_front[0] = (forward_segment1_len - forward_segment0_len) / forward_segment0_len;
  }

  for (std::size_t i = 1; i < num_crack_front_nodes - 1; ++i)
  {
    current_node = getCrackFrontNodePtr(i);
    if (current_node->processor_id() == processor_id())
    {
      disp_current_node(0) = disp_x_var.getNodalValue(*current_node);
      disp_current_node(1) = disp_y_var.getNodalValue(*current_node);
      disp_current_node(2) = disp_z_var.getNodalValue(*current_node);

      previous_node = getCrackFrontNodePtr(i - 1);
      disp_previous_node(0) = disp_x_var.getNodalValue(*previous_node);
      disp_previous_node(1) = disp_y_var.getNodalValue(*previous_node);
      disp_previous_node(2) = disp_z_var.getNodalValue(*previous_node);

      next_node = getCrackFrontNodePtr(i + 1);
      disp_next_node(0) = disp_x_var.getNodalValue(*next_node);
      disp_next_node(1) = disp_y_var.getNodalValue(*next_node);
      disp_next_node(2) = disp_z_var.getNodalValue(*next_node);

      back_segment0 = *current_node - *previous_node;
      back_segment0 = (back_segment0 * _tangent_directions[i]) * _tangent_directions[i];
      back_segment0_len = back_segment0.norm();

      back_segment1 = (*current_node + disp_current_node) - (*previous_node + disp_previous_node);
      back_segment1 = (back_segment1 * _tangent_directions[i]) * _tangent_directions[i];
      back_segment1_len = back_segment1.norm();

      forward_segment0 = *next_node - *current_node;
      forward_segment0 = (forward_segment0 * _tangent_directions[i]) * _tangent_directions[i];
      forward_segment0_len = forward_segment0.norm();

      forward_segment1 = (*next_node + disp_next_node) - (*current_node + disp_current_node);
      forward_segment1 = (forward_segment1 * _tangent_directions[i]) * _tangent_directions[i];
      forward_segment1_len = forward_segment1.norm();

      _strain_along_front[i] =
          0.5 * ((back_segment1_len - back_segment0_len) / back_segment0_len +
                 (forward_segment1_len - forward_segment0_len) / forward_segment0_len);
    }
  }

  current_node = getCrackFrontNodePtr(num_crack_front_nodes - 1);
  if (current_node->processor_id() == processor_id())
  {
    disp_current_node(0) = disp_x_var.getNodalValue(*current_node);
    disp_current_node(1) = disp_y_var.getNodalValue(*current_node);
    disp_current_node(2) = disp_z_var.getNodalValue(*current_node);

    previous_node = getCrackFrontNodePtr(num_crack_front_nodes - 2);
    disp_previous_node(0) = disp_x_var.getNodalValue(*previous_node);
    disp_previous_node(1) = disp_y_var.getNodalValue(*previous_node);
    disp_previous_node(2) = disp_z_var.getNodalValue(*previous_node);

    back_segment0 = *current_node - *previous_node;
    back_segment0 = (back_segment0 * _tangent_directions[num_crack_front_nodes - 1]) *
                    _tangent_directions[num_crack_front_nodes - 1];
    back_segment0_len = back_segment0.norm();

    back_segment1 = (*current_node + disp_current_node) - (*previous_node + disp_previous_node);
    back_segment1 = (back_segment1 * _tangent_directions[num_crack_front_nodes - 1]) *
                    _tangent_directions[num_crack_front_nodes - 1];
    back_segment1_len = back_segment1.norm();

    _strain_along_front[num_crack_front_nodes - 1] =
        (back_segment1_len - back_segment0_len) / back_segment0_len;
  }
}

Real
CrackFrontDefinition::getCrackFrontTangentialStrain(const std::size_t node_index) const
{
  Real strain;
  if (_t_stress)
  {
    strain = _strain_along_front[node_index];
    mooseAssert(strain > -std::numeric_limits<Real>::max(),
                "Failure in parallel communication of crack tangential strain");
  }
  else
    mooseError("In CrackFrontDefinition, tangential strain not available");

  return strain;
}

void
CrackFrontDefinition::createQFunctionRings()
{
  // In the variable names, "cfn" = crack front node

  if (_treat_as_2d && _use_mesh_cutter == false) // 2D: the q-function defines an integral domain
                                                 // that is constant along the crack front
  {
    std::vector<std::vector<const Elem *>> nodes_to_elem_map;
    MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), nodes_to_elem_map);

    std::set<dof_id_type> nodes_prev_ring;
    nodes_prev_ring.insert(_ordered_crack_front_nodes.begin(), _ordered_crack_front_nodes.end());

    std::set<dof_id_type> connected_nodes_this_cfn;
    connected_nodes_this_cfn.insert(_ordered_crack_front_nodes.begin(),
                                    _ordered_crack_front_nodes.end());

    std::set<dof_id_type> old_ring_nodes_this_cfn = connected_nodes_this_cfn;

    // The first ring contains only the crack front node(s)
    std::pair<dof_id_type, std::size_t> node_ring_index =
        std::make_pair(_ordered_crack_front_nodes[0], 1);
    _crack_front_node_to_node_map[node_ring_index].insert(connected_nodes_this_cfn.begin(),
                                                          connected_nodes_this_cfn.end());

    // Build rings of nodes around the crack front node
    for (std::size_t ring = 2; ring <= _last_ring; ++ring)
    {

      // Find nodes connected to the nodes of the previous ring
      std::set<dof_id_type> new_ring_nodes_this_cfn;
      for (auto nit = old_ring_nodes_this_cfn.begin(); nit != old_ring_nodes_this_cfn.end(); ++nit)
      {
        std::vector<const Node *> neighbors;
        MeshTools::find_nodal_neighbors(
            _mesh.getMesh(), _mesh.nodeRef(*nit), nodes_to_elem_map, neighbors);
        for (std::size_t inei = 0; inei < neighbors.size(); ++inei)
        {
          auto thisit = connected_nodes_this_cfn.find(neighbors[inei]->id());

          // Add only nodes that are not already present in any of the rings
          if (thisit == connected_nodes_this_cfn.end())
            new_ring_nodes_this_cfn.insert(neighbors[inei]->id());
        }
      }

      // Add new nodes to rings
      connected_nodes_this_cfn.insert(new_ring_nodes_this_cfn.begin(),
                                      new_ring_nodes_this_cfn.end());
      old_ring_nodes_this_cfn = new_ring_nodes_this_cfn;

      std::pair<dof_id_type, std::size_t> node_ring_index =
          std::make_pair(_ordered_crack_front_nodes[0], ring);
      _crack_front_node_to_node_map[node_ring_index].insert(connected_nodes_this_cfn.begin(),
                                                            connected_nodes_this_cfn.end());
    }
  }
  else // 3D: The q-function defines one integral domain around each crack front node
  {
    std::size_t num_crack_front_points = _ordered_crack_front_nodes.size();
    std::vector<std::vector<const Elem *>> nodes_to_elem_map;
    MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), nodes_to_elem_map);
    for (std::size_t icfn = 0; icfn < num_crack_front_points; ++icfn)
    {
      std::set<dof_id_type> nodes_prev_ring;
      nodes_prev_ring.insert(_ordered_crack_front_nodes[icfn]);

      std::set<dof_id_type> connected_nodes_prev_cfn;
      std::set<dof_id_type> connected_nodes_this_cfn;
      std::set<dof_id_type> connected_nodes_next_cfn;

      connected_nodes_this_cfn.insert(_ordered_crack_front_nodes[icfn]);

      if (_closed_loop && icfn == 0)
      {
        connected_nodes_prev_cfn.insert(_ordered_crack_front_nodes[num_crack_front_points - 1]);
        connected_nodes_next_cfn.insert(_ordered_crack_front_nodes[icfn + 1]);
      }
      else if (_closed_loop && icfn == num_crack_front_points - 1)
      {
        connected_nodes_prev_cfn.insert(_ordered_crack_front_nodes[icfn - 1]);
        connected_nodes_next_cfn.insert(_ordered_crack_front_nodes[0]);
      }
      else if (icfn == 0)
      {
        connected_nodes_next_cfn.insert(_ordered_crack_front_nodes[icfn + 1]);
      }
      else if (icfn == num_crack_front_points - 1)
      {
        connected_nodes_prev_cfn.insert(_ordered_crack_front_nodes[icfn - 1]);
      }
      else
      {
        connected_nodes_prev_cfn.insert(_ordered_crack_front_nodes[icfn - 1]);
        connected_nodes_next_cfn.insert(_ordered_crack_front_nodes[icfn + 1]);
      }

      std::set<dof_id_type> old_ring_nodes_prev_cfn = connected_nodes_prev_cfn;
      std::set<dof_id_type> old_ring_nodes_this_cfn = connected_nodes_this_cfn;
      std::set<dof_id_type> old_ring_nodes_next_cfn = connected_nodes_next_cfn;

      // The first ring contains only the crack front node
      std::pair<dof_id_type, std::size_t> node_ring_index =
          std::make_pair(_ordered_crack_front_nodes[icfn], 1);
      _crack_front_node_to_node_map[node_ring_index].insert(connected_nodes_this_cfn.begin(),
                                                            connected_nodes_this_cfn.end());

      // Build rings of nodes around the crack front node
      for (std::size_t ring = 2; ring <= _last_ring; ++ring)
      {

        // Find nodes connected to the nodes of the previous ring, but exclude nodes in rings of
        // neighboring crack front nodes
        std::set<dof_id_type> new_ring_nodes_this_cfn;
        addNodesToQFunctionRing(new_ring_nodes_this_cfn,
                                old_ring_nodes_this_cfn,
                                connected_nodes_this_cfn,
                                connected_nodes_prev_cfn,
                                connected_nodes_next_cfn,
                                nodes_to_elem_map);

        std::set<dof_id_type> new_ring_nodes_prev_cfn;
        addNodesToQFunctionRing(new_ring_nodes_prev_cfn,
                                old_ring_nodes_prev_cfn,
                                connected_nodes_prev_cfn,
                                connected_nodes_this_cfn,
                                connected_nodes_next_cfn,
                                nodes_to_elem_map);

        std::set<dof_id_type> new_ring_nodes_next_cfn;
        addNodesToQFunctionRing(new_ring_nodes_next_cfn,
                                old_ring_nodes_next_cfn,
                                connected_nodes_next_cfn,
                                connected_nodes_prev_cfn,
                                connected_nodes_this_cfn,
                                nodes_to_elem_map);

        // Add new nodes to the three sets of nodes
        connected_nodes_prev_cfn.insert(new_ring_nodes_prev_cfn.begin(),
                                        new_ring_nodes_prev_cfn.end());
        connected_nodes_this_cfn.insert(new_ring_nodes_this_cfn.begin(),
                                        new_ring_nodes_this_cfn.end());
        connected_nodes_next_cfn.insert(new_ring_nodes_next_cfn.begin(),
                                        new_ring_nodes_next_cfn.end());
        old_ring_nodes_prev_cfn = new_ring_nodes_prev_cfn;
        old_ring_nodes_this_cfn = new_ring_nodes_this_cfn;
        old_ring_nodes_next_cfn = new_ring_nodes_next_cfn;

        std::pair<dof_id_type, std::size_t> node_ring_index =
            std::make_pair(_ordered_crack_front_nodes[icfn], ring);
        _crack_front_node_to_node_map[node_ring_index].insert(connected_nodes_this_cfn.begin(),
                                                              connected_nodes_this_cfn.end());
      }
    }
  }
}

void
CrackFrontDefinition::addNodesToQFunctionRing(
    std::set<dof_id_type> & nodes_new_ring,
    const std::set<dof_id_type> & nodes_old_ring,
    const std::set<dof_id_type> & nodes_all_rings,
    const std::set<dof_id_type> & nodes_neighbor1,
    const std::set<dof_id_type> & nodes_neighbor2,
    std::vector<std::vector<const Elem *>> & nodes_to_elem_map)
{
  for (auto nit = nodes_old_ring.begin(); nit != nodes_old_ring.end(); ++nit)
  {
    std::vector<const Node *> neighbors;
    MeshTools::find_nodal_neighbors(
        _mesh.getMesh(), _mesh.nodeRef(*nit), nodes_to_elem_map, neighbors);
    for (std::size_t inei = 0; inei < neighbors.size(); ++inei)
    {
      auto previt = nodes_all_rings.find(neighbors[inei]->id());
      auto thisit = nodes_neighbor1.find(neighbors[inei]->id());
      auto nextit = nodes_neighbor2.find(neighbors[inei]->id());

      // Add only nodes that are not already present in any of the three sets of nodes
      if (previt == nodes_all_rings.end() && thisit == nodes_neighbor1.end() &&
          nextit == nodes_neighbor2.end())
        nodes_new_ring.insert(neighbors[inei]->id());
    }
  }
}

bool
CrackFrontDefinition::isNodeInRing(const std::size_t ring_index,
                                   const dof_id_type connected_node_id,
                                   const std::size_t node_index) const
{
  bool is_node_in_ring = false;
  std::pair<dof_id_type, std::size_t> node_ring_key =
      std::make_pair(_ordered_crack_front_nodes[node_index], ring_index);
  auto nnmit = _crack_front_node_to_node_map.find(node_ring_key);

  if (nnmit == _crack_front_node_to_node_map.end())
    mooseError("Could not find crack front node ",
               _ordered_crack_front_nodes[node_index],
               " in the crack front node to q-function ring-node map for ring ",
               ring_index);

  std::set<dof_id_type> q_func_nodes = nnmit->second;
  if (q_func_nodes.find(connected_node_id) != q_func_nodes.end())
    is_node_in_ring = true;

  return is_node_in_ring;
}

Real
CrackFrontDefinition::DomainIntegralQFunction(std::size_t crack_front_point_index,
                                              std::size_t ring_index,
                                              const Node * const current_node) const
{
  Real dist_to_crack_front;
  Real dist_along_tangent;
  projectToFrontAtPoint(
      dist_to_crack_front, dist_along_tangent, crack_front_point_index, current_node);

  Real q = 1.0;
  if (dist_to_crack_front > _j_integral_radius_inner[ring_index] &&
      dist_to_crack_front < _j_integral_radius_outer[ring_index])
    q = (_j_integral_radius_outer[ring_index] - dist_to_crack_front) /
        (_j_integral_radius_outer[ring_index] - _j_integral_radius_inner[ring_index]);
  else if (dist_to_crack_front >= _j_integral_radius_outer[ring_index])
    q = 0.0;

  if (q > 0.0)
  {
    Real tangent_multiplier = 1.0;
    if (!_treat_as_2d)
    {
      const Real forward_segment_length =
          getCrackFrontForwardSegmentLength(crack_front_point_index);
      const Real backward_segment_length =
          getCrackFrontBackwardSegmentLength(crack_front_point_index);

      if (dist_along_tangent >= 0.0)
      {
        if (forward_segment_length > 0.0)
          tangent_multiplier = 1.0 - dist_along_tangent / forward_segment_length;
      }
      else
      {
        if (backward_segment_length > 0.0)
          tangent_multiplier = 1.0 + dist_along_tangent / backward_segment_length;
      }
    }

    tangent_multiplier = std::max(tangent_multiplier, 0.0);
    tangent_multiplier = std::min(tangent_multiplier, 1.0);

    // Set to zero if a node is on a designated free surface and its crack front node is not.
    if (isNodeOnIntersectingBoundary(current_node) &&
        !_is_point_on_intersecting_boundary[crack_front_point_index])
      tangent_multiplier = 0.0;

    q *= tangent_multiplier;
  }

  return q;
}

Real
CrackFrontDefinition::DomainIntegralTopologicalQFunction(std::size_t crack_front_point_index,
                                                         std::size_t ring_index,
                                                         const Node * const current_node) const
{
  Real q = 0;
  bool is_node_in_ring = isNodeInRing(ring_index, current_node->id(), crack_front_point_index);
  if (is_node_in_ring)
    q = 1;

  return q;
}

void
CrackFrontDefinition::projectToFrontAtPoint(Real & dist_to_front,
                                            Real & dist_along_tangent,
                                            std::size_t crack_front_point_index,
                                            const Node * const current_node) const
{
  const Point * crack_front_point = getCrackFrontPoint(crack_front_point_index);

  Point p = *current_node;
  const RealVectorValue & crack_front_tangent = getCrackFrontTangent(crack_front_point_index);

  RealVectorValue crack_node_to_current_node = p - *crack_front_point;
  dist_along_tangent = crack_node_to_current_node * crack_front_tangent;
  RealVectorValue projection_point = *crack_front_point + dist_along_tangent * crack_front_tangent;
  RealVectorValue axis_to_current_node = p - projection_point;
  dist_to_front = axis_to_current_node.norm();
}

void
CrackFrontDefinition::isCutterModified(const bool is_cutter_modified)
{
  _is_cutter_modified = is_cutter_modified;
}

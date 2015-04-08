/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CrackFrontDefinition.h"
#include <map>
#include <vector>

template<>
InputParameters validParams<CrackFrontDefinition>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params += validParams<BoundaryRestrictable>();
  addCrackFrontDefinitionParams(params);
  params.set<bool>("use_displaced_mesh") = false;
  return params;
}

void addCrackFrontDefinitionParams(InputParameters& params)
{
  MooseEnum direction_method("CrackDirectionVector CrackMouth CurvedCrackFront");
  MooseEnum end_direction_method("NoSpecialTreatment CrackDirectionVector", "NoSpecialTreatment");
  params.addRequiredParam<MooseEnum>("crack_direction_method", direction_method, "Method to determine direction of crack propagation.  Choices are: " + direction_method.getRawNames());
  params.addParam<MooseEnum>("crack_end_direction_method", end_direction_method, "Method to determine direction of crack propagation at ends of crack.  Choices are: " + end_direction_method.getRawNames());
  params.addParam<RealVectorValue>("crack_direction_vector","Direction of crack propagation");
  params.addParam<RealVectorValue>("crack_direction_vector_end_1","Direction of crack propagation for the node at end 1 of the crack");
  params.addParam<RealVectorValue>("crack_direction_vector_end_2","Direction of crack propagation for the node at end 2 of the crack");
  params.addParam<std::vector<BoundaryName> >("crack_mouth_boundary","Boundaries whose average coordinate defines the crack mouth");
  params.addParam<std::vector<BoundaryName> >("intersecting_boundary","Boundaries intersected by ends of crack");
  params.addParam<bool>("2d", false, "Treat body as two-dimensional");
  params.addRangeCheckedParam<unsigned int>("axis_2d", 2, "axis_2d>=0 & axis_2d<=2", "Out of plane axis for models treated as two-dimensional (0=x, 1=y, 2=z)");
  params.addParam<unsigned int>("symmetry_plane", "Account for a symmetry plane passing through the plane of the crack, normal to the specified axis (0=x, 1=y, 2=z)");
  params.addParam<bool>("t_stress", false, "Calculate T-stress");
  params.addParam<VariableName>("disp_x","Variable containing the x displacement");
  params.addParam<VariableName>("disp_y","Variable containing the y displacement");
  params.addParam<VariableName>("disp_z","Variable containing the z displacement");
}

const Real CrackFrontDefinition::_tol = 1e-14;

CrackFrontDefinition::CrackFrontDefinition(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    BoundaryRestrictable(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_subproblem.mesh()),
    _treat_as_2d(getParam<bool>("2d")),
    _closed_loop(false),
    _axis_2d(getParam<unsigned int>("axis_2d")),
    _has_symmetry_plane(isParamValid("symmetry_plane")),
    _symmetry_plane(_has_symmetry_plane ? getParam<unsigned int>("symmetry_plane") : std::numeric_limits<unsigned int>::max()),
    _t_stress(getParam<bool>("t_stress"))
{
  if (isParamValid("crack_mouth_boundary"))
    _crack_mouth_boundary_names = getParam<std::vector<BoundaryName> >("crack_mouth_boundary");

  if (_has_symmetry_plane)
    if (_symmetry_plane > 2)
      mooseError("symmetry_plane out of bounds: " << _symmetry_plane << " Must be >=0 and <=2.");

  MooseEnum direction_method_moose_enum = getParam<MooseEnum>("crack_direction_method");
  _direction_method = DIRECTION_METHOD(int(direction_method_moose_enum));
  switch (_direction_method)
  {
    case CRACK_DIRECTION_VECTOR:
      if (!isParamValid("crack_direction_vector"))
        mooseError("crack_direction_vector must be specified if crack_direction_method = CrackDirectionVector");
      _crack_direction_vector = getParam<RealVectorValue>("crack_direction_vector");
      break;
    case CRACK_MOUTH:
      if (isParamValid("crack_direction_vector"))
        mooseError("crack_direction_vector must not be specified if crack_direction_method = CrackMouthNodes");
      if (_crack_mouth_boundary_names.size() == 0)
        mooseError("crack_mouth_boundary must be specified if crack_direction_method = CrackMouthNodes");
      break;
    case CURVED_CRACK_FRONT:
      if (isParamValid("crack_direction_vector"))
        mooseError("crack_direction_vector must not be specified if crack_direction_method = CurvedCrackFront");
      break;
    default:
      mooseError("Invalid direction_method");
  }

  if (isParamValid("intersecting_boundary"))
    _intersecting_boundary_names = getParam<std::vector<BoundaryName> >("intersecting_boundary");

  MooseEnum end_direction_method_moose_enum = getParam<MooseEnum>("crack_end_direction_method");
  if (end_direction_method_moose_enum.isValid())
  {
    _end_direction_method = END_DIRECTION_METHOD(int(end_direction_method_moose_enum));
    if (_end_direction_method == END_CRACK_DIRECTION_VECTOR)
    {
      if (!isParamValid("crack_direction_vector_end_1"))
        mooseError("crack_direction_vector_end_1 must be specified if crack_end_direction_method = CrackDirectionVector");
      if (!isParamValid("crack_direction_vector_end_2"))
        mooseError("crack_direction_vector_end_2 must be specified if crack_end_direction_method = CrackDirectionVector");
      _crack_direction_vector_end_1 = getParam<RealVectorValue>("crack_direction_vector_end_1");
      _crack_direction_vector_end_2 = getParam<RealVectorValue>("crack_direction_vector_end_2");
    }
  }

  if (isParamValid("disp_x") && isParamValid("disp_y") && isParamValid("disp_z"))
  {
    _disp_x_var_name = getParam<VariableName>("disp_x");
    _disp_y_var_name = getParam<VariableName>("disp_y");
    _disp_z_var_name = getParam<VariableName>("disp_z");
  }
  else if (_t_stress)
    mooseError("Displacement variables must be provided for T-stress calculation");
}

CrackFrontDefinition::~CrackFrontDefinition()
{
}

void
CrackFrontDefinition::execute()
{
  //Because J-Integral is based on original geometry, the crack front geometry
  //is never updated, so everything that needs to happen is done in initialSetup()
  if (_t_stress)
    calculateTangentialStrainAlongFront();
}

void
CrackFrontDefinition::initialSetup()
{
  _crack_mouth_boundary_ids = _mesh.getBoundaryIDs(_crack_mouth_boundary_names,true);
  _intersecting_boundary_ids = _mesh.getBoundaryIDs(_intersecting_boundary_names,true);

  std::set<dof_id_type> nodes;
  getCrackFrontNodes(nodes);
  orderCrackFrontNodes(nodes);

  updateCrackFrontGeometry();

  if (_t_stress)
  {
    unsigned int num_crack_front_nodes = _ordered_crack_front_nodes.size();
    for (unsigned int i=0; i<num_crack_front_nodes; ++i)
      _strain_along_front.push_back(-std::numeric_limits<Real>::max());
  }
}

void
CrackFrontDefinition::initialize()
{
}

void
CrackFrontDefinition::finalize()
{
  if (_t_stress)
    _communicator.max(_strain_along_front);
}

void
CrackFrontDefinition::getCrackFrontNodes(std::set<dof_id_type>& nodes)
{
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;

    if (hasBoundary(boundary_id))
      nodes.insert(bnode->_node->id());
  }

  if (_treat_as_2d)
  {
    if (nodes.size() > 1)
    {
      //Delete all but one node if they are collinear in the axis normal to the 2d plane
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

      Real node0coor0;
      Real node0coor1;

      for (std::set<dof_id_type>::iterator sit=nodes.begin(); sit != nodes.end(); ++sit)
      {
        Node & curr_node = _mesh.node(*sit);
        if (sit == nodes.begin())
        {
          node0coor0 = curr_node(axis0);
          node0coor1 = curr_node(axis1);
        }
        else
        {
          if (!MooseUtils::absoluteFuzzyEqual(curr_node(axis0), node0coor0, _tol) ||
              !MooseUtils::absoluteFuzzyEqual(curr_node(axis1), node0coor1, _tol))
            mooseError("Boundary provided in CrackFrontDefinition contains " << nodes.size() << " nodes, which are not collinear in the " << _axis_2d << " axis.  Must contain either 1 node or collinear nodes to treat as 2D.");
        }
      }

      std::set<dof_id_type>::iterator second_node = nodes.begin();
      ++second_node;
      nodes.erase(second_node,nodes.end());
    }
  }
}

void
CrackFrontDefinition::orderCrackFrontNodes(std::set<dof_id_type> &nodes)
{
  _ordered_crack_front_nodes.clear();
  if (nodes.size() < 1)
    mooseError("No crack front nodes");
  else if (nodes.size() == 1)
  {
    _ordered_crack_front_nodes.push_back(*nodes.begin());
    if (!_treat_as_2d)
      mooseError("Boundary provided in CrackFrontDefinition contains 1 node, but model is not treated as 2d");
  }
  else // nodes.size() > 1
  {
    if (_treat_as_2d)
      mooseError("Boundary provided in CrackFrontDefinition contains " << nodes.size() << " nodes.  Must contain 1 node to treat as 2D.");

    //Loop through the set of crack front nodes, and create a node to element map for just the crack front nodes
    //The main reason for creating a second map is that we need to do a sort prior to the set_intersection.
    //The original map contains vectors, and we can't sort them, so we create sets in the local map.
    std::map<dof_id_type, std::vector<dof_id_type> > & node_to_elem_map = _mesh.nodeToElemMap();
    std::map<dof_id_type, std::set<dof_id_type> > crack_front_node_to_elem_map;

    for (std::set<dof_id_type>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
    {
      std::map<dof_id_type, std::vector<dof_id_type> >::iterator nemit = node_to_elem_map.find(*nit);
      if (nemit == node_to_elem_map.end())
        mooseError("Could not find crack front node " << *nit << "in the node to elem map");

      std::vector<dof_id_type> & connected_elems = nemit->second;
      for (unsigned int i=0; i<connected_elems.size(); ++i)
        crack_front_node_to_elem_map[*nit].insert(connected_elems[i]);
    }


    //Determine which nodes are connected to each other via elements, and construct line elements to represent
    //those connections
    std::vector<std::vector<dof_id_type> > line_elems;
    std::map<dof_id_type, std::vector<dof_id_type> > node_to_line_elem_map;

    for (std::map<dof_id_type, std::set<dof_id_type> >::iterator cfnemit = crack_front_node_to_elem_map.begin();
         cfnemit != crack_front_node_to_elem_map.end();
         ++cfnemit)
    {
      std::map<dof_id_type, std::set<dof_id_type> >::iterator cfnemit2 = cfnemit;
      for (++cfnemit2;
           cfnemit2 != crack_front_node_to_elem_map.end();
           ++cfnemit2)
      {

        std::vector<dof_id_type> common_elements;
        std::set<dof_id_type> &elements_connected_to_node1 = cfnemit->second;
        std::set<dof_id_type> &elements_connected_to_node2 = cfnemit2->second;
        std::set_intersection(elements_connected_to_node1.begin(), elements_connected_to_node1.end(),
                              elements_connected_to_node2.begin(), elements_connected_to_node2.end(),
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

    //Find nodes on ends of line (those connected to only one line element)
    std::vector<dof_id_type> end_nodes;
    for (std::map<dof_id_type, std::vector<dof_id_type> >::iterator nlemit = node_to_line_elem_map.begin();
         nlemit != node_to_line_elem_map.end();
         ++nlemit)
    {
      unsigned int num_connected_elems = nlemit->second.size();
      if (num_connected_elems == 1)
        end_nodes.push_back(nlemit->first);
      else if (num_connected_elems != 2)
        mooseError("Node " << nlemit->first << " is connected to >2 line segments in CrackFrontDefinition");
    }

    //For embedded crack with closed loop of crack front nodes, must pick the end nodes
    if (end_nodes.size() == 0) //Crack front is a loop.  Pick nodes to be end nodes.
    {
      pickLoopCrackEndNodes(end_nodes, nodes, node_to_line_elem_map, line_elems);
      _closed_loop = true;
      if (_end_direction_method == END_CRACK_DIRECTION_VECTOR)
        mooseError("In CrackFrontDefinition, end_direction_method cannot be CrackDirectionVector for a closed-loop crack");
      if (_intersecting_boundary_names.size() > 0)
        mooseError("In CrackFrontDefinition, intersecting_boundary cannot be specified for a closed-loop crack");
    }
    else if (end_nodes.size() == 2) //Rearrange the order of the end nodes if needed
      orderEndNodes(end_nodes);
    else
      mooseError("In CrackFrontDefinition wrong number of end nodes.  Number end nodes = " << end_nodes.size());

    //Create an ordered list of the nodes going along the line of the crack front
    _ordered_crack_front_nodes.push_back(end_nodes[0]);

    dof_id_type last_node = end_nodes[0];
    dof_id_type second_last_node = last_node;
    while (last_node != end_nodes[1])
    {
      std::vector<dof_id_type> & curr_node_line_elems = node_to_line_elem_map[last_node];
      bool found_new_node = false;
      for (unsigned int i=0; i<curr_node_line_elems.size(); ++i)
      {
        std::vector<dof_id_type> curr_line_elem = line_elems[curr_node_line_elems[i]];
        for (unsigned int j=0; j<curr_line_elem.size(); ++j)
        {
          dof_id_type line_elem_node = curr_line_elem[j];
          if (_closed_loop && (last_node == end_nodes[0] && line_elem_node == end_nodes[1])) //wrong direction around closed loop
            continue;
          if (line_elem_node != last_node &&
              line_elem_node != second_last_node)
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
      last_node = _ordered_crack_front_nodes[_ordered_crack_front_nodes.size()-1];
    }
  }
}

void
CrackFrontDefinition::orderEndNodes(std::vector<dof_id_type> &end_nodes)
{
  //Choose the node to be the first node.  Do that based on undeformed coordinates for repeatability.
  Node & node0 = _mesh.node(end_nodes[0]);
  Node & node1 = _mesh.node(end_nodes[1]);

  unsigned int num_positive_coor0 = 0;
  unsigned int num_positive_coor1 = 0;
  Real dist_from_origin0 = 0.0;
  Real dist_from_origin1 = 0.0;
  for (unsigned int i=0; i<3; ++i)
  {
    dist_from_origin0 += node0(i)*node0(i);
    dist_from_origin1 += node1(i)*node1(i);
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
    unsigned int tmp_node = end_nodes[1];
    end_nodes[1] = end_nodes[0];
    end_nodes[0] = tmp_node;
  }
}

void
CrackFrontDefinition::pickLoopCrackEndNodes(std::vector<dof_id_type> &end_nodes,
                                            std::set<dof_id_type> &nodes,
                                            std::map<dof_id_type, std::vector<dof_id_type> > &node_to_line_elem_map,
                                            std::vector<std::vector<dof_id_type> > &line_elems)
{
  unsigned int max_dist_node;
  Real min_dist = std::numeric_limits<Real>::max();
  Real max_dist = -std::numeric_limits<Real>::max();
  //Pick the node farthest from the origin as the end node, or the one with
  //the greatest x coordinate if the nodes are equidistant from the origin
  for (std::set<dof_id_type>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
  {
    Node & node = _mesh.node(*nit);
    Real dist = node.size();
    if (dist > max_dist)
    {
      max_dist = dist;
      max_dist_node = *nit;
    }
    else if (dist < min_dist)
      min_dist = dist;
  }

  unsigned int end_node;
  if (MooseUtils::absoluteFuzzyGreaterThan(max_dist, min_dist, _tol))
    end_node = max_dist_node;
  else
  {
    std::vector<Node *> node_vec;
    for (std::set<dof_id_type>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
      node_vec.push_back(_mesh.nodePtr(*nit));
    end_node = maxNodeCoor(node_vec);
  }

  end_nodes.push_back(end_node);

  //Find the two nodes connected to the node identified as the end node, and pick one of those to be the other end node
  std::vector<dof_id_type> end_node_line_elems = node_to_line_elem_map[end_node];
  if (end_node_line_elems.size() != 2)
    mooseError("Crack front nodes are in a loop, but crack end node is only connected to one other node");
  std::vector<Node *> candidate_other_end_nodes;

  for (unsigned int i=0; i<2; ++i)
  {
    std::vector<dof_id_type> end_line_elem = line_elems[end_node_line_elems[i]];
    for (unsigned int j=0; j<end_line_elem.size(); ++j)
    {
      unsigned int line_elem_node = end_line_elem[j];
      if (line_elem_node != end_node)
        candidate_other_end_nodes.push_back(_mesh.nodePtr(line_elem_node));
    }
  }
  if (candidate_other_end_nodes.size() != 2)
    mooseError("Crack front nodes are in a loop, but crack end node is not connected to two other nodes");
  end_nodes.push_back(maxNodeCoor(candidate_other_end_nodes,1));
}

unsigned int
CrackFrontDefinition::maxNodeCoor(std::vector<Node *>& nodes, unsigned int dir0)
{
  Real dirs[3];
  if (dir0 == 0)
  {
    dirs[0]=0;
    dirs[1]=1;
    dirs[2]=2;
  }
  else if (dir0 == 1)
  {
    dirs[0]=1;
    dirs[1]=2;
    dirs[2]=0;
  }
  else if (dir0 == 2)
  {
    dirs[0]=2;
    dirs[1]=0;
    dirs[2]=1;
  }
  else
    mooseError("Invalid dir0 in CrackFrontDefinition::maxNodeCoor()");

  Real max_coor0 = -std::numeric_limits<Real>::max();
  std::vector<Node *> max_coor0_nodes;
  for (unsigned int i=0; i<nodes.size(); ++i)
  {
    Real coor0 = (*nodes[i])(dirs[0]);
    if (coor0 > max_coor0)
      max_coor0 = coor0;
  }
  for (unsigned int i=0; i<nodes.size(); ++i)
  {
    Real coor0 = (*nodes[i])(dirs[0]);
    if (MooseUtils::absoluteFuzzyEqual(coor0, max_coor0, _tol))
      max_coor0_nodes.push_back(nodes[i]);
  }
  if (max_coor0_nodes.size() > 1)
  {
    Real max_coor1 = -std::numeric_limits<Real>::max();
    std::vector<Node *> max_coor1_nodes;
    for (unsigned int i=0; i<nodes.size(); ++i)
    {
      Real coor1 = (*nodes[i])(dirs[1]);
      if (coor1 > max_coor1)
        max_coor1 = coor1;
    }
    for (unsigned int i=0; i<nodes.size(); ++i)
    {
      Real coor1 = (*nodes[i])(dirs[1]);
      if (MooseUtils::absoluteFuzzyEqual(coor1, max_coor1, _tol))
        max_coor1_nodes.push_back(nodes[i]);
    }
    if (max_coor1_nodes.size() > 1)
    {
      Real max_coor2 = -std::numeric_limits<Real>::max();
      std::vector<Node *> max_coor2_nodes;
      for (unsigned int i=0; i<nodes.size(); ++i)
      {
        Real coor2 = (*nodes[i])(dirs[2]);
        if (coor2 > max_coor2)
          max_coor2 = coor2;
      }
      for (unsigned int i=0; i<nodes.size(); ++i)
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
  _distances_along_front.clear();
  _angles_along_front.clear();
  _tangent_directions.clear();
  _crack_directions.clear();
  _rot_matrix.clear();

  if (_treat_as_2d)
  {
    RealVectorValue tangent_direction;
    RealVectorValue crack_direction;
    tangent_direction(_axis_2d) = 1.0;
    _tangent_directions.push_back(tangent_direction);
    const Node* crack_front_node = _mesh.nodePtr(_ordered_crack_front_nodes[0]);
    crack_direction = calculateCrackFrontDirection(crack_front_node,tangent_direction,MIDDLE_NODE);
    _crack_directions.push_back(crack_direction);
    _crack_plane_normal = tangent_direction.cross(crack_direction);
    ColumnMajorMatrix rot_mat;
    rot_mat(0,0) = crack_direction(0);
    rot_mat(0,1) = crack_direction(1);
    rot_mat(0,2) = crack_direction(2);
    rot_mat(1,0) = _crack_plane_normal(0);
    rot_mat(1,1) = _crack_plane_normal(1);
    rot_mat(1,2) = _crack_plane_normal(2);
    rot_mat(2,0) = 0.0;
    rot_mat(2,1) = 0.0;
    rot_mat(2,2) = 0.0;
    rot_mat(2,_axis_2d) = 1.0;
    _rot_matrix.push_back(rot_mat);

    _segment_lengths.push_back(std::make_pair(0.0,0.0));
    _distances_along_front.push_back(0.0);
    _angles_along_front.push_back(0.0);
    _overall_length = 0.0;
  }
  else
  {
    unsigned int num_crack_front_nodes = _ordered_crack_front_nodes.size();
    std::vector<Node*> crack_front_nodes;
    crack_front_nodes.reserve(num_crack_front_nodes);
    _segment_lengths.reserve(num_crack_front_nodes);
    _tangent_directions.reserve(num_crack_front_nodes);
    _crack_directions.reserve(num_crack_front_nodes);
    for (unsigned int i=0; i<num_crack_front_nodes; ++i)
    {
      crack_front_nodes.push_back(_mesh.nodePtr(_ordered_crack_front_nodes[i]));
    }

    _overall_length = 0.0;

    RealVectorValue back_segment;
    Real back_segment_len = 0.0;
    if (_closed_loop)
    {
      back_segment = *crack_front_nodes[0] - *crack_front_nodes[num_crack_front_nodes-1];
      back_segment_len = back_segment.size();
    }

    for (unsigned int i=0; i<num_crack_front_nodes; ++i)
    {
      CRACK_NODE_TYPE ntype;
      if (_closed_loop)
        ntype = MIDDLE_NODE;
      else if (i==0)
        ntype = END_1_NODE;
      else if (i==num_crack_front_nodes-1)
        ntype = END_2_NODE;
      else
        ntype = MIDDLE_NODE;

      RealVectorValue forward_segment;
      Real forward_segment_len;
      if (ntype == END_2_NODE)
        forward_segment_len = 0.0;
      else if (_closed_loop && i==num_crack_front_nodes-1)
      {
        forward_segment = *crack_front_nodes[0] - *crack_front_nodes[i];
        forward_segment_len = forward_segment.size();
      }
      else
      {
        forward_segment = *crack_front_nodes[i+1] - *crack_front_nodes[i];
        forward_segment_len = forward_segment.size();
        _overall_length += forward_segment_len;
      }

      _segment_lengths.push_back(std::make_pair(back_segment_len,forward_segment_len));
      if (i == 0)
        _distances_along_front.push_back(0.0);
      else
        _distances_along_front.push_back(back_segment_len + _distances_along_front[i - 1]);

      RealVectorValue tangent_direction = back_segment + forward_segment;
      tangent_direction = tangent_direction / tangent_direction.size();
      _tangent_directions.push_back(tangent_direction);
      _crack_directions.push_back(calculateCrackFrontDirection(crack_front_nodes[i],tangent_direction,ntype));


      back_segment = forward_segment;
      back_segment_len = forward_segment_len;
    }

    //For CURVED_CRACK_FRONT, _crack_plane_normal gets computed in updateDataForCrackDirection
    if (_direction_method != CURVED_CRACK_FRONT)
    {
      unsigned int mid_id = (num_crack_front_nodes - 1) / 2;
      _crack_plane_normal = _tangent_directions[mid_id].cross(_crack_directions[mid_id]);

      //Make sure the normal vector is non-zero
      RealVectorValue zero_vec(0.0);
      if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
        mooseError("Crack plane normal vector evaluates to zero");
    }

    //Calculate angles of each point along the crack front for an elliptical crack projected
    //to a circle.
    if (hasAngleAlongFront())
    {
      RealVectorValue origin_to_first_node = *crack_front_nodes[0] - _crack_mouth_coordinates;
      Real hyp = origin_to_first_node.size();
      RealVectorValue norm_origin_to_first_node = origin_to_first_node / hyp;
      RealVectorValue tangent_to_first_node = -norm_origin_to_first_node.cross(_crack_plane_normal);
      tangent_to_first_node /= tangent_to_first_node.size();

      for (unsigned int i=0; i<num_crack_front_nodes; ++i)
      {
        RealVectorValue origin_to_curr_node = *crack_front_nodes[i] - _crack_mouth_coordinates;

        Real adj = origin_to_curr_node*norm_origin_to_first_node;
        Real opp = origin_to_curr_node*tangent_to_first_node;

        Real angle = acos(adj / hyp) * 180.0 / libMesh::pi;
        if (opp < 0.0)
          angle = 360.0 - angle;
        _angles_along_front.push_back(angle);
      }

      //Correct angle on end nodes if they are 0 or 360 to be consistent with neighboring node
      if (num_crack_front_nodes > 1)
      {
        if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[0], 0.0, _tol) &&
            _angles_along_front[1] > 180.0)
          _angles_along_front[0] = 360.0;
        else if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[0], 360.0, _tol) &&
            _angles_along_front[1] < 180.0)
          _angles_along_front[0] = 0.0;

        if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[num_crack_front_nodes - 1], 0.0, _tol) &&
            _angles_along_front[num_crack_front_nodes - 2] > 180.0)
          _angles_along_front[num_crack_front_nodes - 1] = 360.0;
        else if (MooseUtils::absoluteFuzzyEqual(_angles_along_front[num_crack_front_nodes - 1], 360.0, _tol) &&
                 _angles_along_front[num_crack_front_nodes - 2] < 180.0)
          _angles_along_front[num_crack_front_nodes - 1] = 0.0;
      }
    }
    else
      _angles_along_front.resize(num_crack_front_nodes,0.0);

    // Create rotation matrix
    for (unsigned int i=0; i<num_crack_front_nodes; ++i)
    {
      ColumnMajorMatrix rot_mat;
      rot_mat(0,0) = _crack_directions[i](0);
      rot_mat(0,1) = _crack_directions[i](1);
      rot_mat(0,2) = _crack_directions[i](2);
      rot_mat(1,0) = _crack_plane_normal(0);
      rot_mat(1,1) = _crack_plane_normal(1);
      rot_mat(1,2) = _crack_plane_normal(2);
      rot_mat(2,0) = _tangent_directions[i](0);
      rot_mat(2,1) = _tangent_directions[i](1);
      rot_mat(2,2) = _tangent_directions[i](2);
      _rot_matrix.push_back(rot_mat);
    }

    _console << "Summary of J-Integral crack front geometry:" << std::endl;
    _console << "index   node id   x coord       y coord       z coord       x dir         y dir          z dir        angle        position     seg length" << std::endl;
    for (unsigned int i=0; i<crack_front_nodes.size(); ++i)
    {
      _console << std::left
               << std::setw(8) << i + 1
               << std::setw(10) << crack_front_nodes[i]->id()
               << std::setw(14) << (*crack_front_nodes[i])(0)
               << std::setw(14) << (*crack_front_nodes[i])(1)
               << std::setw(14) << (*crack_front_nodes[i])(2)
               << std::setw(14) << _crack_directions[i](0)
               << std::setw(14) << _crack_directions[i](1)
               << std::setw(14) << _crack_directions[i](2);
      if (hasAngleAlongFront())
        _console << std::left << std::setw(14) << _angles_along_front[i];
      else
        _console << std::left << std::setw(14) << "--";
      _console << std::left
               << std::setw(14) << _distances_along_front[i]
               << std::setw(14) << (_segment_lengths[i].first + _segment_lengths[i].second) / 2.0
               << std::endl;
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

    std::set<Node*> crack_mouth_nodes;
    ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
    for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
    {
      const BndNode * bnode = *nd;
      BoundaryID boundary_id = bnode->_bnd_id;

      for (unsigned int ibid=0; ibid<_crack_mouth_boundary_ids.size(); ++ibid)
      {
        if (boundary_id == _crack_mouth_boundary_ids[ibid])
        {
          crack_mouth_nodes.insert(bnode->_node);
          break;
        }
      }
    }

    for (std::set<Node*>::iterator nit=crack_mouth_nodes.begin();
         nit != crack_mouth_nodes.end();
         ++nit)
    {
      _crack_mouth_coordinates += **nit;
    }
    _crack_mouth_coordinates /= (Real)crack_mouth_nodes.size();

    if (_has_symmetry_plane)
      _crack_mouth_coordinates(_symmetry_plane) = 0.0;
  }

  if (_direction_method == CURVED_CRACK_FRONT)
  {
    _crack_plane_normal.zero();

    //Get 3 nodes on crack front
    unsigned int num_nodes(_ordered_crack_front_nodes.size());
    if (num_nodes<3)
    {
      mooseError("Crack front must contain at least 3 nodes to use CurvedCrackFront option");
    }
    unsigned int start_id;
    unsigned int mid_id;
    unsigned int end_id;

    if (_closed_loop)
    {
      start_id = 0;
      mid_id = (num_nodes-1)/3;
      end_id = 2*mid_id;
    }
    else
    {
      start_id = 0;
      mid_id = (num_nodes-1)/2;
      end_id = num_nodes-1;
    }
    Node & start = _mesh.node(_ordered_crack_front_nodes[start_id]);
    Node & mid   = _mesh.node(_ordered_crack_front_nodes[mid_id]);
    Node & end   = _mesh.node(_ordered_crack_front_nodes[end_id]);

    //Create two vectors connecting them
    RealVectorValue v1 = mid-start;
    RealVectorValue v2 = end-mid;

    //Take cross product to get normal
    _crack_plane_normal = v1.cross(v2);
    _crack_plane_normal = _crack_plane_normal.unit();

    //Make sure they're not collinear
    RealVectorValue zero_vec(0.0);
    if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
    {
      mooseError("Nodes on crack front are too close to being collinear");
    }
  }
}

RealVectorValue
CrackFrontDefinition::calculateCrackFrontDirection(const Node* crack_front_node,
                                                   const RealVectorValue& tangent_direction,
                                                   const CRACK_NODE_TYPE ntype) const
{
  RealVectorValue crack_dir;
  RealVectorValue zero_vec(0.0);

  bool calc_dir = true;
  if (_end_direction_method == END_CRACK_DIRECTION_VECTOR)
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
    if (_direction_method == CRACK_DIRECTION_VECTOR)
    {
      crack_dir = _crack_direction_vector;
    }
    else if (_direction_method == CRACK_MOUTH)
    {
      if (_crack_mouth_coordinates.absolute_fuzzy_equals(*crack_front_node, _tol))
      {
        mooseError("Crack mouth too close to crack front node");
      }
      RealVectorValue mouth_to_front = *crack_front_node - _crack_mouth_coordinates;

      RealVectorValue crack_plane_normal = mouth_to_front.cross(tangent_direction);
      if (crack_plane_normal.absolute_fuzzy_equals(zero_vec, _tol))
      {
        mooseError("Vector from crack mouth to crack front node is collinear with crack front segment");
      }

      crack_dir = tangent_direction.cross(crack_plane_normal);
      Real dotprod = crack_dir*mouth_to_front;
      if (dotprod < 0)
      {
        crack_dir = -crack_dir;
      }
    }
    else if (_direction_method == CURVED_CRACK_FRONT)
    {
      crack_dir = tangent_direction.cross(_crack_plane_normal);
    }
  }
  crack_dir = crack_dir.unit();

  return crack_dir;
}

const Node *
CrackFrontDefinition::getCrackFrontNodePtr(const unsigned int node_index) const
{
  mooseAssert(node_index < _ordered_crack_front_nodes.size(),"node_index out of range");
  const Node * crack_front_node = _mesh.nodePtr(_ordered_crack_front_nodes[node_index]);
  mooseAssert(crack_front_node != NULL,"invalid crack front node");
  return crack_front_node;
}

const RealVectorValue &
CrackFrontDefinition::getCrackFrontTangent(const unsigned int node_index) const
{
  mooseAssert(node_index < _ordered_crack_front_nodes.size(),"node_index out of range");
  return _tangent_directions[node_index];
}

Real
CrackFrontDefinition::getCrackFrontForwardSegmentLength(const unsigned int node_index) const
{
  return _segment_lengths[node_index].second;
}

Real
CrackFrontDefinition::getCrackFrontBackwardSegmentLength(const unsigned int node_index) const
{
  return _segment_lengths[node_index].first;
}

const RealVectorValue &
CrackFrontDefinition::getCrackDirection(const unsigned int node_index) const
{
  return _crack_directions[node_index];
}

Real
CrackFrontDefinition::getDistanceAlongFront(const unsigned int node_index) const
{
  return _distances_along_front[node_index];
}

bool
CrackFrontDefinition::hasAngleAlongFront() const
{
  return (_crack_mouth_boundary_names.size() > 0);
}

Real
CrackFrontDefinition::getAngleAlongFront(const unsigned int node_index) const
{
  if (!hasAngleAlongFront())
    mooseError("In CrackFrontDefinition, Requested angle along crack front, but not available.  Must specify crack_mouth_boundary.");
  return _angles_along_front[node_index];
}

unsigned int
CrackFrontDefinition::getNumCrackFrontNodes() const
{
  return _ordered_crack_front_nodes.size();
}

RealVectorValue
CrackFrontDefinition::rotateToCrackFrontCoords(const RealVectorValue vector, const unsigned int node_index) const
{
  ColumnMajorMatrix vec3x1;
  vec3x1 = _rot_matrix[node_index] * vector;
  RealVectorValue vec;
  vec(0) = vec3x1(0,0);
  vec(1) = vec3x1(1,0);
  vec(2) = vec3x1(2,0);
  return vec;
}

ColumnMajorMatrix
CrackFrontDefinition::rotateToCrackFrontCoords(const SymmTensor tensor, const unsigned int node_index) const
{
  ColumnMajorMatrix tensor_CMM;
  tensor_CMM(0,0) = tensor.xx();
  tensor_CMM(0,1) = tensor.xy();
  tensor_CMM(0,2) = tensor.xz();
  tensor_CMM(1,0) = tensor.xy();
  tensor_CMM(1,1) = tensor.yy();
  tensor_CMM(1,2) = tensor.yz();
  tensor_CMM(2,0) = tensor.xz();
  tensor_CMM(2,1) = tensor.yz();
  tensor_CMM(2,2) = tensor.zz();

  ColumnMajorMatrix tmp = _rot_matrix[node_index] * tensor_CMM;
  ColumnMajorMatrix rotT = _rot_matrix[node_index].transpose();
  ColumnMajorMatrix rotated_tensor = tmp * rotT;

  return rotated_tensor;
}

ColumnMajorMatrix
CrackFrontDefinition::rotateToCrackFrontCoords(const ColumnMajorMatrix tensor, const unsigned int node_index) const
{
  ColumnMajorMatrix tmp = _rot_matrix[node_index] * tensor;
  ColumnMajorMatrix rotT = _rot_matrix[node_index].transpose();
  ColumnMajorMatrix rotated_tensor = tmp * rotT;

  return rotated_tensor;
}

void
CrackFrontDefinition::calculateRThetaToCrackFront(const Point qp, const unsigned int node_index, Real & r, Real & theta) const
{
  unsigned int num_nodes(_ordered_crack_front_nodes.size());
  Point p = qp;
  Point closest_node(0.0);
  RealVectorValue closest_node_to_p;

  Node & crack_tip_node = _mesh.node(_ordered_crack_front_nodes[node_index]);
  RealVectorValue crack_tip_node_rot = rotateToCrackFrontCoords(crack_tip_node,node_index);

  RealVectorValue crack_front_edge = rotateToCrackFrontCoords(_tangent_directions[node_index],node_index);

  Point p_rot = rotateToCrackFrontCoords(p,node_index);
  p_rot = p_rot - crack_tip_node_rot;

  if (_treat_as_2d)
  {
    //In 2D, the closest node is the crack tip node and the position of the crack tip node is (0,0,0) in the crack front coordinate system
    //In case this is a 3D mesh treated as 2D, project point onto same plane as crack front node.
    //Note: In the crack front coordinate system, z is always in the tangent direction to the crack front
    p_rot(2) = closest_node(2);
    closest_node_to_p = p_rot;

    //Find r, the distance between the qp and the crack front
    RealVectorValue r_vec = p_rot;
    r = r_vec.size();

  }
  else
  {
    // Loop over crack front nodes to find the one closest to the point qp
    Real min_dist = std::numeric_limits<Real>::max();
    for (unsigned int nit = 0; nit != num_nodes; ++nit)
    {
      Node & crack_front_node = _mesh.node(_ordered_crack_front_nodes[nit]);
      RealVectorValue crack_node_to_current_node = p - crack_front_node;
      Real dist = crack_node_to_current_node.size();

      if (dist < min_dist)
      {
        min_dist = dist;
        closest_node = crack_front_node;
      }
    }

    //Rotate coordinates to crack front coordinate system
    closest_node = rotateToCrackFrontCoords(closest_node,node_index);
    closest_node = closest_node - crack_tip_node_rot;

    //Find r, the distance between the qp and the crack front
    Real edge_length_sq = crack_front_edge.size_sq();
    closest_node_to_p = p_rot - closest_node;
    Real perp = crack_front_edge * closest_node_to_p;
    Real dist_along_edge = perp / edge_length_sq;
    RealVectorValue point_on_edge = closest_node + crack_front_edge * dist_along_edge;
    RealVectorValue r_vec = p_rot - point_on_edge;
    r = r_vec.size();

  }

  //Find theta, the angle between r and the crack front plane
  RealVectorValue crack_plane_normal = rotateToCrackFrontCoords(_crack_plane_normal,node_index);
  Real p_to_plane_dist = std::abs(closest_node_to_p*crack_plane_normal);

  //Determine if p is above or below the crack plane
  Real y_local = p_rot(1) - closest_node(1);

  //Determine if p is in front of or behind the crack front
  RealVectorValue p2(p_rot);
  p2(1) = 0;
  RealVectorValue p2_vec = p2 - closest_node;
  Real ahead = crack_front_edge(2) * p2_vec(0) - crack_front_edge(0) * p2_vec(2);

  Real x_local(0);
  if (ahead >= 0)
    x_local = 1;
  else
    x_local = -1;

  //Calculate theta based on in which quadrant in the crack front coordinate
  //system the qp is located
  if (r > 0)
  {
    if (x_local >= 0 && y_local >= 0)
      theta = std::asin(p_to_plane_dist/r);

    else if (x_local < 0 && y_local >= 0)
      theta = libMesh::pi - std::asin(p_to_plane_dist/r);

    else if (x_local < 0 && y_local < 0)
      theta = -(libMesh::pi - std::asin(p_to_plane_dist/r));

    else if (x_local >= 0 && y_local < 0)
      theta = -std::asin(p_to_plane_dist/r);
  }
  else if (r == 0)
    theta = 0;
}

bool
CrackFrontDefinition::isNodeOnIntersectingBoundary(const Node * const node) const
{
  bool is_on_boundary = false;
  mooseAssert(node,"Invalid node");
  dof_id_type node_id = node->id();
  for (unsigned int i=0; i<_intersecting_boundary_ids.size(); ++i)
  {
    if (_mesh.isBoundaryNode(node_id,_intersecting_boundary_ids[i]))
    {
      is_on_boundary = true;
      break;
    }
  }
  return is_on_boundary;
}

void
CrackFrontDefinition::calculateTangentialStrainAlongFront()
{
  RealVectorValue disp_current_node;
  RealVectorValue disp_previous_node;
  RealVectorValue disp_next_node;
  RealVectorValue l0;
  RealVectorValue l1;
  RealVectorValue delta_l0;
  RealVectorValue delta_l1;

  unsigned int num_crack_front_nodes = _ordered_crack_front_nodes.size();
  const Node * current_node;
  const Node * previous_node;
  const Node * next_node;

  // In finalize(), gatherMax builds and distributes the complete strain vector on all processors
  // -> reset the vector every time
  for (unsigned int i=0; i<num_crack_front_nodes; ++i)
    _strain_along_front[i] = -std::numeric_limits<Real>::max();

  current_node = getCrackFrontNodePtr(0);
  if (current_node->processor_id() == processor_id())
  {
    disp_current_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*current_node);
    disp_current_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*current_node);
    disp_current_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*current_node);

    next_node = getCrackFrontNodePtr(1);
    disp_next_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*next_node);
    disp_next_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*next_node);
    disp_next_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*next_node);

    //Calculate change in length of crack front edge and project in the tangent direction to get tangential strain
    l1 = *next_node - *current_node;
    l1 = (l1 * _tangent_directions[0]) * _tangent_directions[0];
    delta_l1 = disp_next_node - disp_current_node;
    delta_l1 = (delta_l1 * _tangent_directions[0]) * _tangent_directions[0];
    _strain_along_front[0] = delta_l1.size() / l1.size();
  }

  for (unsigned int i=1; i<num_crack_front_nodes-1; ++i)
  {
    current_node = getCrackFrontNodePtr(i);
    if (current_node->processor_id() == processor_id())
    {
      disp_current_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*current_node);
      disp_current_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*current_node);
      disp_current_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*current_node);

      previous_node = getCrackFrontNodePtr(i-1);
      disp_previous_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*previous_node);
      disp_previous_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*previous_node);
      disp_previous_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*previous_node);

      next_node = getCrackFrontNodePtr(i+1);
      disp_next_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*next_node);
      disp_next_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*next_node);
      disp_next_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*next_node);

      l0 = *current_node - *previous_node;
      l0 = (l0 * _tangent_directions[i]) * _tangent_directions[i];
      delta_l0 = disp_current_node - disp_previous_node;
      delta_l0 = (delta_l0 * _tangent_directions[i]) * _tangent_directions[i];
      l1 = *next_node - *current_node;
      l1 = (l1 * _tangent_directions[i]) * _tangent_directions[i];
      delta_l1 = disp_next_node - disp_current_node;
      delta_l1 = (delta_l1 * _tangent_directions[i]) * _tangent_directions[i];
      _strain_along_front[i] = 0.5 * ( delta_l0.size()/l0.size() + delta_l1.size()/l1.size() );
    }
  }

  current_node = getCrackFrontNodePtr(num_crack_front_nodes-1);
  if (current_node->processor_id() == processor_id())
  {
    disp_current_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*current_node);
    disp_current_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*current_node);
    disp_current_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*current_node);

    previous_node = getCrackFrontNodePtr(num_crack_front_nodes-2);
    disp_previous_node(0) = _subproblem.getVariable(_tid, _disp_x_var_name).getNodalValue(*previous_node);
    disp_previous_node(1) = _subproblem.getVariable(_tid, _disp_y_var_name).getNodalValue(*previous_node);
    disp_previous_node(2) = _subproblem.getVariable(_tid, _disp_z_var_name).getNodalValue(*previous_node);

    l0 = *current_node - *previous_node;
    delta_l0 = disp_current_node - disp_previous_node;
    l0 = (l0 * _tangent_directions[num_crack_front_nodes-1]) * _tangent_directions[num_crack_front_nodes-1];
    delta_l0 = (delta_l0 * _tangent_directions[num_crack_front_nodes-1]) * _tangent_directions[num_crack_front_nodes-1];

    _strain_along_front[num_crack_front_nodes-1] = delta_l0.size() / l0.size();
  }

}

Real
CrackFrontDefinition::getCrackFrontTangentialStrain(const unsigned int node_index) const
{
  Real strain;
  if (_t_stress)
    strain = _strain_along_front[node_index];
  else
    mooseError("In CrackFrontDefinition, tangential strain not available");

  return strain;
}

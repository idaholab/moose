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
  MooseEnum direction_method("CrackDirectionVector, CrackMouth, CurvedCrackFront");
  MooseEnum end_direction_method("NoSpecialTreatment, CrackDirectionVector", "NoSpecialTreatment");
  params.addRequiredParam<MooseEnum>("crack_direction_method", direction_method, "Method to determine direction of crack propagation.  Choices are: " + direction_method.getRawNames());
  params.addParam<MooseEnum>("crack_end_direction_method", end_direction_method, "Method to determine direction of crack propagation at ends of crack.  Choices are: " + end_direction_method.getRawNames());
  params.addParam<RealVectorValue>("crack_direction_vector","Direction of crack propagation");
  params.addParam<RealVectorValue>("crack_direction_vector_end_1","Direction of crack propagation for the node at end 1 of the crack");
  params.addParam<RealVectorValue>("crack_direction_vector_end_2","Direction of crack propagation for the node at end 2 of the crack");
  params.addParam<std::vector<BoundaryName> >("crack_mouth_boundary","Boundaries whose average coordinate defines the crack mouth");
  params.addParam<bool>("2d", false, "Treat body as two-dimensional");
  params.addRangeCheckedParam<unsigned int>("axis_2d", 2, "axis_2d>=0 & axis_2d<=2", "Out of plane axis for models treated as two-dimensional (0=x, 1=y, 2=z)");
}

const Real CrackFrontDefinition::_tol = 1e-14;

CrackFrontDefinition::CrackFrontDefinition(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    BoundaryRestrictable(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_subproblem.mesh()),
    _treat_as_2d(getParam<bool>("2d")),
    _axis_2d(getParam<unsigned int>("axis_2d"))
{
  MooseEnum direction_method_moose_enum = getParam<MooseEnum>("crack_direction_method");
  if (direction_method_moose_enum.isValid())
  {
    _direction_method = DIRECTION_METHOD(int(direction_method_moose_enum));
    if (_direction_method == CRACK_DIRECTION_VECTOR)
    {
      if (!isParamValid("crack_direction_vector"))
      {
        mooseError("crack_direction_vector must be specified if crack_direction_method = CrackDirectionVector");
      }
      if (isParamValid("crack_mouth_boundary"))
      {
        mooseError("crack_mouth_boundary must not be specified if crack_direction_method = CrackDirectionVector");
      }
      _crack_direction_vector = getParam<RealVectorValue>("crack_direction_vector");
    }
    else if (_direction_method == CRACK_MOUTH)
    {
      if (isParamValid("crack_direction_vector"))
      {
        mooseError("crack_direction_vector must not be specified if crack_direction_method = CrackMouthNodes");
      }
      if (!isParamValid("crack_mouth_boundary"))
      {
        mooseError("crack_mouth_boundary must be specified if crack_direction_method = CrackMouthNodes");
      }
      _crack_mouth_boundary_names = getParam<std::vector<BoundaryName> >("crack_mouth_boundary");
    }
    else if (_direction_method == CURVED_CRACK_FRONT)
    {
      if (isParamValid("crack_direction_vector"))
      {
        mooseError("crack_direction_vector must not be specified if crack_direction_method = CurvedCrackFront");
      }
      if (isParamValid("crack_mouth_boundary"))
      {
        mooseError("crack_mouth_boundary must not be specified if crack_direction_method = CurvedCrackFront");
      }
    }
  }

  MooseEnum end_direction_method_moose_enum = getParam<MooseEnum>("crack_end_direction_method");
  if (end_direction_method_moose_enum.isValid())
  {
    _end_direction_method = END_DIRECTION_METHOD(int(end_direction_method_moose_enum));
    if (_end_direction_method == END_CRACK_DIRECTION_VECTOR)
    {
      if (!isParamValid("crack_direction_vector_end_1"))
      {
        mooseError("crack_direction_vector_end_1 must be specified if crack_end_direction_method = CrackDirectionVector");
      }
      if (!isParamValid("crack_direction_vector_end_2"))
      {
        mooseError("crack_direction_vector_end_2 must be specified if crack_end_direction_method = CrackDirectionVector");
      }
      _crack_direction_vector_end_1 = getParam<RealVectorValue>("crack_direction_vector_end_1");
      _crack_direction_vector_end_2 = getParam<RealVectorValue>("crack_direction_vector_end_2");
    }
  }
}

CrackFrontDefinition::~CrackFrontDefinition()
{
}

void
CrackFrontDefinition::execute()
{
  //Because J-Integral is based on original geometry, the crack front geometry
  //is never updated, so everything that needs to happen is done in initialSetup()
}

void
CrackFrontDefinition::initialSetup()
{
  _crack_mouth_boundary_ids = _mesh.getBoundaryIDs(_crack_mouth_boundary_names,true);

  std::set<unsigned int> nodes;
  getCrackFrontNodes(nodes);
  orderCrackFrontNodes(nodes);

  updateCrackFrontGeometry();
}

void
CrackFrontDefinition::initialize()
{
}

void
CrackFrontDefinition::finalize()
{
}

void
CrackFrontDefinition::threadJoin(const UserObject & /*uo*/)
{
}

void
CrackFrontDefinition::getCrackFrontNodes(std::set<unsigned int>& nodes)
{
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;

    if (hasBoundary(boundary_id))
    {
      nodes.insert(bnode->_node->id());
    }
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
      }

      Real node0coor0;
      Real node0coor1;

      for (std::set<unsigned int>::iterator sit=nodes.begin(); sit != nodes.end(); ++sit)
      {
        Node & curr_node = _mesh.node(*sit);
        if (sit == nodes.begin())
        {
          node0coor0 = curr_node(axis0);
          node0coor1 = curr_node(axis1);
        }
        else
        {
          if ((std::abs(curr_node(axis0) - node0coor0) > _tol) ||
              (std::abs(curr_node(axis1) - node0coor1) > _tol))
          {
            mooseError("Boundary provided in CrackFrontDefinition contains "<<nodes.size()<<" nodes, which are not collinear in the "<<_axis_2d<<" axis.  Must contain either 1 node or collinear nodes to treat as 2D.");
          }
        }
      }

      std::set<unsigned int>::iterator second_node = nodes.begin();
      ++second_node;
      nodes.erase(second_node,nodes.end());
    }
  }
}

void
CrackFrontDefinition::orderCrackFrontNodes(std::set<unsigned int> &nodes)
{
  _ordered_crack_front_nodes.clear();
  if (nodes.size() < 1)
  {
    mooseError("No crack front nodes");
  }
  else if (nodes.size() == 1)
  {
    _ordered_crack_front_nodes.push_back(*nodes.begin());
    if (!_treat_as_2d)
      mooseError("Boundary provided in CrackFrontDefinition contains 1 node, but model is not treated as 2d");
  }
  else // nodes.size() > 1
  {
    if (_treat_as_2d)
      mooseError("Boundary provided in CrackFrontDefinition contains "<<nodes.size()<<" nodes.  Must contain 1 node to treat as 2D.");

    //Loop through the set of crack front nodes, and create a node to element map for just the crack front nodes
    //The main reason for creating a second map is that we need to do a sort prior to the set_intersection.
    //The original map contains vectors, and we can't sort them, so we create sets in the local map.
    std::map<unsigned int, std::vector<unsigned int> > & node_to_elem_map = _mesh.nodeToElemMap();
    std::map<unsigned int, std::set<unsigned int> > crack_front_node_to_elem_map;
    std::map<unsigned int, std::vector<unsigned int> >::iterator nemit;

    for (std::set<unsigned int>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
    {
      nemit = node_to_elem_map.find(*nit);
      if (nemit == node_to_elem_map.end())
        mooseError("Could not find crack front node "<<*nit<<"in the node to elem map");

      std::vector<unsigned int> & connected_elems = nemit->second;
      for (unsigned int i=0; i<connected_elems.size(); ++i)
      {
        crack_front_node_to_elem_map[*nit].insert(connected_elems[i]);
      }
    }


    //Determine which nodes are connected to each other via elements, and construct line elements to represent
    //those connections
    std::vector<std::vector<unsigned int> > line_elems;
    std::map<unsigned int, std::vector<unsigned int> > node_to_line_elem_map;

    for (std::map<unsigned int, std::set<unsigned int> >::iterator cfnemit = crack_front_node_to_elem_map.begin();
         cfnemit != crack_front_node_to_elem_map.end();
         ++cfnemit)
    {
      std::map<unsigned int, std::set<unsigned int> >::iterator cfnemit2 = cfnemit;
      for (++cfnemit2;
           cfnemit2 != crack_front_node_to_elem_map.end();
           ++cfnemit2)
      {

        std::vector<unsigned int> common_elements;
        std::set<unsigned int> &elements_connected_to_node1 = cfnemit->second;
        std::set<unsigned int> &elements_connected_to_node2 = cfnemit2->second;
        std::set_intersection(elements_connected_to_node1.begin(), elements_connected_to_node1.end(),
                              elements_connected_to_node2.begin(), elements_connected_to_node2.end(),
                              std::inserter(common_elements,common_elements.end()));

        if (common_elements.size() > 0)
        {
          std::vector<unsigned int> my_line_elem;
          my_line_elem.push_back(cfnemit->first);
          my_line_elem.push_back(cfnemit2->first);
          node_to_line_elem_map[cfnemit->first].push_back(line_elems.size());
          node_to_line_elem_map[cfnemit2->first].push_back(line_elems.size());
          line_elems.push_back(my_line_elem);
        }
      }
    }

    //Find nodes on ends of line (those connected to only one line element)
    std::vector<unsigned int> end_nodes;
    for (std::map<unsigned int, std::vector<unsigned int> >::iterator nlemit = node_to_line_elem_map.begin();
         nlemit != node_to_line_elem_map.end();
         ++nlemit)
    {
      unsigned int num_connected_elems = nlemit->second.size();
      if (num_connected_elems == 1)
      {
        end_nodes.push_back(nlemit->first);
      }
      else if (num_connected_elems != 2)
      {
        mooseError("Node "<<nlemit->first<<" is connected to >2 line segments in CrackFrontDefinition");
      }

    }

    //For embedded crack with closed loop of crack front nodes, must pick the end nodes
    if (end_nodes.size() == 0) //Crack front is a loop.  Pick nodes to be end nodes.
      pickLoopCrackEndNodes(end_nodes, nodes, node_to_line_elem_map, line_elems);
    else if (end_nodes.size() == 2) //Rearrange the order of the end nodes if needed
      orderEndNodes(end_nodes);
    else
      mooseError("In CrackFrontDefinition wrong number of end nodes.  Number end nodes = "<<end_nodes.size());

    //Create an ordered list of the nodes going along the line of the crack front
    _ordered_crack_front_nodes.push_back(end_nodes[0]);

    unsigned int last_node = end_nodes[0];
    unsigned int second_last_node = last_node;
    while (last_node != end_nodes[1])
    {
      std::vector<unsigned int> & curr_node_line_elems = node_to_line_elem_map[last_node];
      bool found_new_node = false;
      for (unsigned int i=0; i<curr_node_line_elems.size(); ++i)
      {
        std::vector<unsigned int> curr_line_elem = line_elems[curr_node_line_elems[i]];
        for (unsigned int j=0; j<curr_line_elem.size(); ++j)
        {
          unsigned int line_elem_node = curr_line_elem[j];
          if (last_node == end_nodes[0] && line_elem_node == end_nodes[1]) //wrong direction around closed loop
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
        {
          break;
        }
      }
      second_last_node = last_node;
      last_node = _ordered_crack_front_nodes[_ordered_crack_front_nodes.size()-1];
    }
  }
}

void
CrackFrontDefinition::orderEndNodes(std::vector<unsigned int> &end_nodes)
{
  //Choose the node to be the first node.  Do that based on undeformed coordinates for repeatability.
  Node & node0 = _mesh.node(end_nodes[0]);
  Node & node1 = _mesh.node(end_nodes[1]);

  unsigned int num_pos_coor0 = 0;
  unsigned int num_pos_coor1 = 0;
  Real dist_from_origin0 = 0.0;
  Real dist_from_origin1 = 0.0;
  for (unsigned int i=0; i<3; ++i)
  {
    dist_from_origin0 += node0(i)*node0(i);
    dist_from_origin1 += node1(i)*node1(i);
    if (node0(i) > _tol)
    {
      ++num_pos_coor0;
    }
    if (node1(i) > _tol)
    {
      ++num_pos_coor1;
    }
  }
  dist_from_origin0 = std::sqrt(dist_from_origin0);
  dist_from_origin1 = std::sqrt(dist_from_origin1);

  bool switch_ends = false;
  if (num_pos_coor1 > num_pos_coor0)
  {
    switch_ends = true;
  }
  else
  {
    if (std::abs(dist_from_origin1 - dist_from_origin0) > _tol)
    {
      if (dist_from_origin1 < dist_from_origin0)
      {
        switch_ends = true;
      }
    }
    else
    {
      if (end_nodes[1] < end_nodes[0])
      {
        switch_ends = true;
      }
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
CrackFrontDefinition::pickLoopCrackEndNodes(std::vector<unsigned int> &end_nodes,
                                            std::set<unsigned int> &nodes,
                                            std::map<unsigned int, std::vector<unsigned int> > &node_to_line_elem_map,
                                            std::vector<std::vector<unsigned int> > &line_elems)
{
  unsigned int max_dist_node;
  Real min_dist = std::numeric_limits<Real>::max();
  Real max_dist = -std::numeric_limits<Real>::max();
  //Pick the node farthest from the origin as the end node, or the one with
  //the greatest x coordinate if the nodes are equidistant from the origin
  for (std::set<unsigned int>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
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
  if (max_dist - min_dist > _tol)
    end_node = max_dist_node;
  else
  {
    std::vector<Node *> node_vec;
    for (std::set<unsigned int>::iterator nit = nodes.begin(); nit != nodes.end(); ++nit )
      node_vec.push_back(_mesh.nodePtr(*nit));
    end_node = maxNodeCoor(node_vec);
  }

  end_nodes.push_back(end_node);

  //Find the two nodes connected to the node identified as the end node, and pick one of those to be the other end node
  std::vector<unsigned int> end_node_line_elems = node_to_line_elem_map[end_node];
  if (end_node_line_elems.size() != 2)
    mooseError("Crack front nodes are in a loop, but crack end node is only connected to one other node");
  std::vector<Node *> candidate_other_end_nodes;

  for (unsigned int i=0; i<2; ++i)
  {
    std::vector<unsigned int> end_line_elem = line_elems[end_node_line_elems[i]];
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
    if (std::abs(coor0 - max_coor0) <= _tol)
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
      if (std::abs(coor1 - max_coor1) <= _tol)
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
        if (std::abs(coor2 - max_coor2) <= _tol)
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
    _crack_plane_normal = crack_direction.cross(tangent_direction);
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
    for (unsigned int i=0; i<num_crack_front_nodes; ++i)
    {
      CRACK_NODE_TYPE ntype = MIDDLE_NODE;
      if (i==0)
      {
        ntype = END_1_NODE;
      }
      else if (i==num_crack_front_nodes-1)
      {
        ntype = END_2_NODE;
      }

      RealVectorValue forward_segment;
      Real forward_segment_len = 0.0;
      if (ntype != END_2_NODE)
      {
        forward_segment = *crack_front_nodes[i+1] - *crack_front_nodes[i];
        forward_segment_len = forward_segment.size();
      }

      _segment_lengths.push_back(std::make_pair(back_segment_len,forward_segment_len));
      //Moose::out<<"seg len: "<<back_segment_len<<" "<<forward_segment_len<<std::endl;

      RealVectorValue tangent_direction = back_segment + forward_segment;
      tangent_direction = tangent_direction / tangent_direction.size();
      _tangent_directions.push_back(tangent_direction);
      //Moose::out<<"tan dir: "<<tangent_direction(0)<<" "<<tangent_direction(1)<<" "<<tangent_direction(2)<<std::endl;
      _crack_directions.push_back(calculateCrackFrontDirection(crack_front_nodes[i],tangent_direction,ntype));

      _overall_length += forward_segment_len;

      back_segment = forward_segment;
      back_segment_len = forward_segment_len;

    }

    //For CURVED_CRACK_FRONT, _crack_plane_normal gets computed in updateDataForCrackDirection
    if (_direction_method != CURVED_CRACK_FRONT)
    {
      unsigned int mid_id = (num_crack_front_nodes-1)/2;
      _crack_plane_normal = _tangent_directions[mid_id].cross(_crack_directions[mid_id]);

      //Make sure the normal vector is non-zero
      RealVectorValue zero_vec(0.0);
      if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec,1.e-15))
        mooseError("Crack plane normal vector evaluates to zero");
    }

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

    Moose::out<<"Summary of J-Integral crack front geometry:"<<std::endl;
    Moose::out<<"index   node id   x coord       y coord       z coord       x dir         y dir          z dir        seg length"<<std::endl;
    for (unsigned int i=0; i<crack_front_nodes.size(); ++i)
    {
      Moose::out<<std::left
                <<std::setw(8) <<i+1
                <<std::setw(10)<<crack_front_nodes[i]->id()
                <<std::setw(14)<<(*crack_front_nodes[i])(0)
                <<std::setw(14)<<(*crack_front_nodes[i])(1)
                <<std::setw(14)<<(*crack_front_nodes[i])(2)
                <<std::setw(14)<<_crack_directions[i](0)
                <<std::setw(14)<<_crack_directions[i](1)
                <<std::setw(14)<<_crack_directions[i](2)
                <<std::setw(14)<<(_segment_lengths[i].first+_segment_lengths[i].second)/2.0
                <<std::endl;
    }
    Moose::out<<"overall length: "<<_overall_length<<std::endl;
  }
}

void
CrackFrontDefinition::updateDataForCrackDirection()
{
  if (_direction_method == CRACK_MOUTH)
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
  }
  else if (_direction_method == CURVED_CRACK_FRONT)
  {
    _crack_plane_normal.zero();

    //Get 3 nodes on crack front
    unsigned int num_nodes(_ordered_crack_front_nodes.size());
    if (num_nodes<3)
    {
      mooseError("Crack front must contain at least 3 nodes to use CurvedCrackFront option");
    }
    unsigned int mid_id = (num_nodes-1)/2;
    Node & start = _mesh.node(_ordered_crack_front_nodes[0]);
    Node & mid   = _mesh.node(_ordered_crack_front_nodes[mid_id]);
    Node & end   = _mesh.node(_ordered_crack_front_nodes[num_nodes-1]);

    //Create two vectors connecting them
    RealVectorValue v1 = mid-start;
    RealVectorValue v2 = end-mid;

    //Take cross product to get normal
    _crack_plane_normal = v1.cross(v2);
    _crack_plane_normal = _crack_plane_normal.unit();

    //Make sure they're not collinear
    RealVectorValue zero_vec(0.0);
    if (_crack_plane_normal.absolute_fuzzy_equals(zero_vec,1.e-15))
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
      if (_crack_mouth_coordinates.absolute_fuzzy_equals(*crack_front_node,1.e-15))
      {
        mooseError("Crack mouth too close to crack front node");
      }
      RealVectorValue mouth_to_front = *crack_front_node - _crack_mouth_coordinates;

      RealVectorValue crack_plane_normal = mouth_to_front.cross(tangent_direction);
      if (crack_plane_normal.absolute_fuzzy_equals(zero_vec,1.e-15))
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

const Node &
CrackFrontDefinition::getCrackFrontNode(const unsigned int node_index) const
{
  return _mesh.node(_ordered_crack_front_nodes[node_index]);
}

const RealVectorValue &
CrackFrontDefinition::getCrackFrontTangent(const unsigned int node_index) const
{
  return _tangent_directions[node_index];
}

const RealVectorValue &
CrackFrontDefinition::getCrackFrontNormal() const
{
  return _crack_plane_normal;
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

  // Loop over nodes to find the two crack front nodes closest to the point qp
  Real mindist1(1.0e30);
  Real mindist2(1.0e30);
  Point closest_node1(0.0);
  Point closest_node2(0.0);
  for (unsigned int nit = 0; nit != num_nodes; ++nit)
  {
    Node & crack_front_node = _mesh.node(_ordered_crack_front_nodes[nit]);
    RealVectorValue crack_node_to_current_node = p - crack_front_node;
    Real dist = crack_node_to_current_node.size();

    if (dist < mindist1)
    {
      mindist2 = mindist1;
      closest_node2 = closest_node1;
      mindist1 = dist;
      closest_node1 = crack_front_node;
    }
    else if (dist < mindist2 && dist != mindist1)
    {
      mindist2 = dist;
      closest_node2 = crack_front_node;
    }

  }

  //Rotate coordinates to crack front coordinate system
  closest_node1 = rotateToCrackFrontCoords(closest_node1,node_index);
  closest_node2 = rotateToCrackFrontCoords(closest_node2,node_index);
  if (closest_node1(2) > closest_node2(2))
  {
    RealVectorValue tmp = closest_node2;
    closest_node2 = closest_node1;
    closest_node1 = tmp;
  }
  p = rotateToCrackFrontCoords(p,node_index);

  //Find r, the distance between the qp and the crack front
  RealVectorValue crack_front_edge = closest_node2 - closest_node1;
  Real edge_length_sq = crack_front_edge.size_sq();
  RealVectorValue closest_node1_to_p = p - closest_node1;
  Real perp = crack_front_edge * closest_node1_to_p;
  Real dist_along_edge = perp / edge_length_sq;
  RealVectorValue point_on_edge = closest_node1 + crack_front_edge * dist_along_edge;
  RealVectorValue r_vec = p - point_on_edge;
  r = r_vec.size();

  //Find theta, the angle between r and the crack front plane
  RealVectorValue crack_plane_normal = rotateToCrackFrontCoords(_crack_plane_normal,node_index);
  Real p_to_plane_dist = std::abs(closest_node1_to_p*crack_plane_normal);

  //Determine if p is above or below the crack plane
  Real y_local = p(1) - closest_node1(1);
  //Determine if p is in front of or behind the crack front
  RealVectorValue p2(p);
  p2(1) = 0;
  RealVectorValue p2_vec = p2 - closest_node1;
  Real ahead = crack_front_edge(2) * p2_vec(0) - crack_front_edge(0) * p2_vec(2);
  Real x_local(0);
  if (ahead >= 0)
    x_local = 1;
  else
    x_local = -1;

  //Calculate theta based on in which quadrant in the crack front coordinate
  //system the qp is located
  if (x_local >= 0 && y_local >= 0)
    theta = std::asin(p_to_plane_dist/r);

  else if (x_local < 0 && y_local >= 0)
    theta = libMesh::pi - std::asin(p_to_plane_dist/r);

  else if (x_local < 0 && y_local < 0)
    theta = -(libMesh::pi - std::asin(p_to_plane_dist/r));

  else if (x_local >= 0 && y_local < 0)
    theta = -std::asin(p_to_plane_dist/r);

}

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
  MooseEnum direction_method("CrackDirectionVector=1, CrackMouth, CurvedCrackFront");
  params.addRequiredParam<MooseEnum>("crack_direction_method", direction_method, "Method to determine direction of crack propagation.  Choices are: " + direction_method.getRawNames());
  params.addParam<RealVectorValue>("crack_direction_vector","Direction of crack propagation");
  params.addParam<std::vector<BoundaryName> >("crack_mouth_boundary","Boundaries whose average coordinate defines the crack mouth");
  params.addParam<bool>("2d", false, "Treat body as two-dimensional");
  params.addParam<unsigned int>("2d_axis", 2, "Out of plane axis for models treated as two-dimensional (0=x, 1=y, 2=z)");
}

CrackFrontDefinition::CrackFrontDefinition(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    BoundaryRestrictable(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_subproblem.mesh()),
    _treat_as_2d(getParam<bool>("2d")),
    _axis_2d(getParam<unsigned int>("2d_axis"))
{
  MooseEnum direction_method_moose_enum = getParam<MooseEnum>("crack_direction_method");
  if (direction_method_moose_enum.isValid())
  {
    _direction_method = CDM_ENUM(int(direction_method_moose_enum));
    if (_direction_method == CDM_CRACK_DIRECTION_VECTOR)
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
    else if (_direction_method == CDM_CRACK_MOUTH)
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
    else if (_direction_method == CDM_CURVED_CRACK_FRONT)
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

//  Moose::out<<"Ordered nodes: ";
//  for (unsigned int i=0; i<_ordered_crack_front_nodes.size(); ++i)
//  {
//    Moose::out<<_ordered_crack_front_nodes[i]<<" ";
//  }
//  Moose::out<<std::endl;

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
CrackFrontDefinition::threadJoin(const UserObject & uo)
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
}

void
CrackFrontDefinition::orderCrackFrontNodes(std::set<unsigned int> nodes)
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
      mooseError("Nodeset provided in CrackFrontDefinition contains 1 node, but model is not treated as 2d");
  }
  else // nodes.size() > 1
  {
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

    if (end_nodes.size() != 2)
    {
      mooseError("In CrackFrontDefinition number of end nodes != 2.  Number end nodes = "<<end_nodes.size());
    }

    //Rearrange the order of the end nodes if needed
    orderEndNodes(end_nodes);

    //Create an ordered list of the nodes going along the line of the crack front
    _ordered_crack_front_nodes.push_back(end_nodes[0]);

    if (_treat_as_2d)
    {
      mooseWarning("Nodeset provided in CrackFrontDefinition contains multiple nodes, but model treated as 2d. Using node "<<_ordered_crack_front_nodes[0]);
    }
    else
    {
      unsigned int last_node = end_nodes[0];
      unsigned int second_last_node = last_node;
      while(last_node != end_nodes[1])
      {
        std::vector<unsigned int> & curr_node_line_elems = node_to_line_elem_map[last_node];
        bool found_new_node = false;
        for (unsigned int i=0; i<curr_node_line_elems.size(); ++i)
        {
          std::vector<unsigned int> curr_line_elem = line_elems[curr_node_line_elems[i]];
          for (unsigned int j=0; j<curr_line_elem.size(); ++j)
          {
            unsigned int line_elem_node = curr_line_elem[j];
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
}

void
CrackFrontDefinition::orderEndNodes(std::vector<unsigned int> &end_nodes)
{
  //Choose the node to be the first node.  Do that based on undeformed coordinates for repeatability.
  Node & node0 = _mesh.node(end_nodes[0]);
  Node & node1 = _mesh.node(end_nodes[1]);
  Real tol = 1e-14;

  unsigned int num_pos_coor0 = 0;
  unsigned int num_pos_coor1 = 0;
  Real dist_from_origin0 = 0.0;
  Real dist_from_origin1 = 0.0;
  for (unsigned int i=0; i<3; ++i)
  {
    dist_from_origin0 += node0(i)*node0(i);
    dist_from_origin1 += node1(i)*node1(i);
    if (node0(i) > tol)
    {
      ++num_pos_coor0;
    }
    if (node1(i) > tol)
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
    if (std::abs(dist_from_origin1 - dist_from_origin0) > tol)
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
CrackFrontDefinition::updateCrackFrontGeometry()
{
  updateCrackDirectionCoords();

  _segment_lengths.clear();
  _tangent_directions.clear();
  _crack_directions.clear();

  if (_treat_as_2d)
  {
    RealVectorValue tangent_direction;
    tangent_direction(_axis_2d) = 1.0;
    _tangent_directions.push_back(tangent_direction);
    const Node* crack_front_node = _mesh.nodePtr(_ordered_crack_front_nodes[0]);
    _crack_directions.push_back(calculateCrackFrontDirection(crack_front_node,tangent_direction));
    _segment_lengths.push_back(std::make_pair(0.0,0.0));
    _overall_length = 0.0;
  }
  else
  {
    std::vector<Node*> crack_front_nodes;
    crack_front_nodes.reserve(_ordered_crack_front_nodes.size());
    _segment_lengths.reserve(_ordered_crack_front_nodes.size());
    _tangent_directions.reserve(_ordered_crack_front_nodes.size());
    _crack_directions.reserve(_ordered_crack_front_nodes.size());
    for (unsigned int i=0; i<_ordered_crack_front_nodes.size(); ++i)
    {
      crack_front_nodes.push_back(_mesh.nodePtr(_ordered_crack_front_nodes[i]));
    }

    _overall_length = 0.0;

    RealVectorValue back_segment;
    Real back_segment_len = 0.0;
    for (unsigned int i=0; i<crack_front_nodes.size(); ++i)
    {
      RealVectorValue forward_segment;
      Real forward_segment_len = 0.0;
      if (i!=crack_front_nodes.size()-1)
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
      _crack_directions.push_back(calculateCrackFrontDirection(crack_front_nodes[i],tangent_direction));

      _overall_length += forward_segment_len;

      back_segment = forward_segment;
      back_segment_len = forward_segment_len;
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
CrackFrontDefinition::updateCrackDirectionCoords()
{
  if (_direction_method == CDM_CRACK_MOUTH)
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
  else if (_direction_method == CDM_CURVED_CRACK_FRONT)
  {
    _crack_plane_normal_from_curved_front.zero();

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
    _crack_plane_normal_from_curved_front = v1.cross(v2);

    //Make sure they're not collinear
    RealVectorValue zero_vec(0.0);
    if (_crack_plane_normal_from_curved_front.absolute_fuzzy_equals(zero_vec,1.e-15))
    {
      mooseError("Nodes on crack front are too close to being collinear");
    }
  }
}

RealVectorValue
CrackFrontDefinition::calculateCrackFrontDirection(const Node* crack_front_node,
                                                   const RealVectorValue& tangent_direction) const
{
  RealVectorValue crack_dir;
  RealVectorValue zero_vec(0.0);
  if (_direction_method == CDM_CRACK_DIRECTION_VECTOR)
  {
    crack_dir = _crack_direction_vector;
  }
  else if (_direction_method == CDM_CRACK_MOUTH)
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
    crack_dir = crack_dir.unit();
    Real dotprod = crack_dir*mouth_to_front;
    if (dotprod < 0)
    {
      crack_dir = -crack_dir;
    }
  }
  else if (_direction_method == CDM_CURVED_CRACK_FRONT)
  {
    crack_dir = tangent_direction.cross(_crack_plane_normal_from_curved_front);
    crack_dir = crack_dir.unit();
  }
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

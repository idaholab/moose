#include "CrackFrontDefinition.h"
#include <map>
#include <vector>

template<>
InputParameters validParams<CrackFrontDefinition>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params += validParams<BoundaryRestrictable>();
  params.addRequiredParam<RealVectorValue>("crack_direction","Direction of crack propagation");
  params.addParam<bool>("2d", false, "Treat body as two-dimensional");
  params.addParam<unsigned int>("2d_axis", 2, "Out of plane axis for models treated as two-dimensional (0=x, 1=y, 2=z)");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

CrackFrontDefinition::CrackFrontDefinition(const std::string & name, InputParameters parameters) :
    GeneralUserObject(name, parameters),
    BoundaryRestrictable(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _mesh(_subproblem.mesh()),
    _crack_direction(getParam<RealVectorValue>("crack_direction")),
    _treat_as_2d(getParam<bool>("2d")),
    _axis_2d(getParam<unsigned int>("2d_axis"))
{
}

CrackFrontDefinition::~CrackFrontDefinition()
{
}

void
CrackFrontDefinition::execute()
{
  updateCrackFrontGeometry();
}

void
CrackFrontDefinition::initialSetup()
{
  //TODO:  This is an attempt to get the coordinates updated for the off-processor nodes.  It still doesn't work.
  for(std::set<BoundaryID>::iterator biditer=_bnd_ids.begin(); biditer != _bnd_ids.end(); ++biditer)
  {
    _subproblem.addGhostedBoundary(*biditer);
  }

  std::set<unsigned int> nodes;
  ConstBndNodeRange & bnd_nodes = *_mesh.getBoundaryNodeRange();
  for (ConstBndNodeRange::const_iterator nd = bnd_nodes.begin() ; nd != bnd_nodes.end(); ++nd)
  {
    const BndNode * bnode = *nd;
    BoundaryID boundary_id = bnode->_bnd_id;

    if (_bnd_ids.find(boundary_id) != _bnd_ids.end())
    {
      nodes.insert(bnode->_node->id());
    }
  }

  orderCrackFrontNodes(nodes);
  updateCrackFrontGeometry();

//  std::cout<<"Ordered nodes: ";
//  for (unsigned int i=0; i<_ordered_crack_front_nodes.size(); ++i)
//  {
//    std::cout<<_ordered_crack_front_nodes[i]<<" ";
//  }
//  std::cout<<std::endl;

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
  if (_treat_as_2d)
  {
    RealVectorValue tangent_direction;
    tangent_direction(_axis_2d) = 1.0;
    _tangent_directions.push_back(tangent_direction);
    _segment_lengths.push_back(std::make_pair(0.0,0.0));
    _overall_length = 0.0;
  }
  else
  {
    std::vector<Node*> crack_front_nodes;
    crack_front_nodes.reserve(_ordered_crack_front_nodes.size());
    _segment_lengths.clear();
    _segment_lengths.reserve(_ordered_crack_front_nodes.size());
    _tangent_directions.clear();
    _tangent_directions.reserve(_ordered_crack_front_nodes.size());
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
      //    std::cout<<"seg len: "<<back_segment_len<<" "<<forward_segment_len<<std::endl;

      RealVectorValue tangent_direction = back_segment + forward_segment;
      tangent_direction = tangent_direction / tangent_direction.size();
      _tangent_directions.push_back(tangent_direction);
      //    std::cout<<"tan dir: "<<tangent_direction(0)<<" "<<tangent_direction(1)<<" "<<tangent_direction(2)<<std::endl;

      _overall_length += forward_segment_len;

      back_segment = forward_segment;
      back_segment_len = forward_segment_len;
    }
    //  std::cout<<"overall len: "<<_overall_length<<std::endl;
  }
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
  return _crack_direction;
}

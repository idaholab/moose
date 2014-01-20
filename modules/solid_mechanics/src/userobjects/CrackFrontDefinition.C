#include "CrackFrontDefinition.h"
#include <map>
#include <vector>

template<>
InputParameters validParams<CrackFrontDefinition>()
{
  InputParameters params = validParams<NodalUserObject>();
  return params;
}

CrackFrontDefinition::CrackFrontDefinition(const std::string & name, InputParameters parameters) :
    NodalUserObject(name, parameters),
    _aux(_fe_problem.getAuxiliarySystem())
{
}

CrackFrontDefinition::~CrackFrontDefinition()
{
}

void
CrackFrontDefinition::execute()
{
  _nodes.insert(_current_node->id());
}

void
CrackFrontDefinition::initialize()
{
}

void
CrackFrontDefinition::finalize()
{
  Parallel::set_union(_nodes);

  orderCrackFrontNodes();

//  std::cout<<"Ordered nodes: ";
//  for (unsigned int i=0; i<_ordered_crack_front_nodes.size(); ++i)
//  {
//    std::cout<<_ordered_crack_front_nodes[i]<<" ";
//  }
//  std::cout<<std::endl;

}

void
CrackFrontDefinition::threadJoin(const UserObject & uo)
{
  const CrackFrontDefinition & cfd = dynamic_cast<const CrackFrontDefinition &>(uo);

  for (std::set<unsigned int>::const_iterator it = cfd._nodes.begin();
       it != cfd._nodes.end();
       ++it )
  {
    _nodes.insert(*it);
  }
}

void
CrackFrontDefinition::orderCrackFrontNodes()
{
  //Loop through the set of crack front nodes, and create a node to element map for just the crack front nodes
  //The main reason for creating a second map is that we need to do a sort prior to the set_intersection.
  //The original map contains vectors, and we can't sort them, so we create sets in the local map.
  std::map<unsigned int, std::vector<unsigned int> > & node_to_elem_map = _mesh.nodeToElemMap();
  std::map<unsigned int, std::set<unsigned int> > crack_front_node_to_elem_map;
  std::map<unsigned int, std::vector<unsigned int> >::iterator nemit;

  for (std::set<unsigned int>::iterator nit = _nodes.begin(); nit != _nodes.end(); ++nit )
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
  _ordered_crack_front_nodes.clear();
  _ordered_crack_front_nodes.push_back(end_nodes[0]);

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

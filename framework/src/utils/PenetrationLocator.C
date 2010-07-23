#include "PenetrationLocator.h"

#include "boundary_info.h"
#include "elem.h"
//#include "plane.h"

PenetrationLocator::PenetrationLocator(Mesh & mesh, short int master, short int slave)
  : _mesh(mesh),
    _master_boundary(master),
    _slave_boundary(slave)
{}


void
PenetrationLocator::detectPenetration()
{
  // Data structures to hold the element boundary information
  std::vector< unsigned int > elem_list;
  std::vector< unsigned short int > side_list;
  std::vector< short int > id_list;

  // Retrieve the Element Boundary data structures from the mesh
  _mesh.boundary_info->build_side_list(elem_list, side_list, id_list);

  // Data strcutres to hold the Nodal Boundary conditions
  std::vector< unsigned int > node_list;
  std::vector< short int > node_boundary_list;
  _mesh.boundary_info->build_node_list_from_side_list();
  _mesh.boundary_info->build_node_list(node_list, node_boundary_list);

  // Iterator pairs for use in range functions
  std::pair<std::vector<short int>::iterator, std::vector<short int>::iterator> bounds_node_boundary;
  std::pair<std::vector<unsigned int>::iterator, std::vector<unsigned int>::iterator> bounds_nodes, bounds_elems;
  std::pair<std::vector<unsigned short int>::iterator, std::vector<unsigned short int>::iterator> bounds_sides;

  // Find the iterators for the master sideset nodes
  bounds_node_boundary = std::equal_range(node_boundary_list.begin(), node_boundary_list.end(), _master_boundary);

  // Bound the vector returned from the boundary_info class where the corresponding id vector matches the
  // master boundary.  We are going to see if any of the nodes on this boundary have penetrated through
  // elements on the slave boundary
  bounds_nodes = std::make_pair(
    node_list.begin() + int(bounds_node_boundary.first - node_boundary_list.begin()),
    node_list.begin() + int(bounds_node_boundary.second - node_boundary_list.begin()));

  MeshBase::const_node_iterator node = _mesh.local_nodes_begin();
  MeshBase::const_node_iterator end_node = _mesh.local_nodes_end();
  for ( ; node != end_node ; ++node)
  {
    // This is a node on the master boundary
    if (std::binary_search(bounds_nodes.first, bounds_nodes.second, (*node)->id()))
    {
      // Now we need to loop over the active elements to see if we have penetrated any
      // TODO: This might need some cleanup in parallel
      MeshBase::const_element_iterator el = _mesh.active_local_elements_begin();
      MeshBase::const_element_iterator end_el = _mesh.active_local_elements_end();
      for ( ; el != end_el ; ++el)
      {
        const Elem* elem = *el;

        bounds_node_boundary = std::equal_range(id_list.begin(), id_list.end(), _slave_boundary);

        bounds_elems = std::make_pair(
          elem_list.begin() + int(bounds_node_boundary.first - id_list.begin()),
          elem_list.begin() + int(bounds_node_boundary.second - id_list.begin()));
        
        std::vector<unsigned int>::iterator pos = std::lower_bound (bounds_elems.first,
                                                                    bounds_elems.second,
                                                                    elem->id());

        // Found the local element on the slave boundary
        if (pos != bounds_elems.second && *pos == elem->id())
        {
          if (elem->contains_point(**node))
          {
            unsigned int side_num = *(side_list.begin() + int(pos - elem_list.begin()));

#ifdef DEBUG            
            std::cout << "Node " << (*node)->id() << " contained in " << elem->id()
                      << " through side " << side_num
                      << ". Distance: " << normDistance(*(elem->build_side(side_num)), **node) << "\n";
#endif            

            _penetrated_elems[(*node)->id()] = std::make_pair(elem->id(), normDistance(*(elem->build_side(side_num)), **node));
            
          }
        }
      }
    }
  }
}

Real
PenetrationLocator::penetrationDistance(unsigned int node_id) const
{
  std::map<unsigned int, std::pair<unsigned int, Real> >::const_iterator found_it;
  
  if ((found_it = _penetrated_elems.find(node_id)) != _penetrated_elems.end())
    return found_it->second.second;
  else
    return 0;
}

Real
PenetrationLocator::normDistance(const Elem & side, const Point & p0)
{
  Real d;
  unsigned int dim = _mesh.mesh_dimension();

  if (dim == 2)
  {
//    libmesh_assert(side->n_points() == 2);

    Point p1 = side.point(0);
    Point p2 = side.point(1);

//    std::cerr << "\nLine Segment: (" << p1(0) << "," << p1(1) << ") - (" << p2(0) << "," << p2(1) << ") "
//ç              << "Point: (" << p0(0) << "," << p0(1) << ")\n";
    
    
    d = std::sqrt(std::pow(((p2(1)-p1(1))*(p0(0)-p1(0))-(p2(0)-p1(0))*(p0(1)-p1(1))),2) /
                   (std::pow(p2(0)-p1(0),2) + std::pow(p2(1)-p1(1),2)));
  }
  else if (dim == 3)
  {
//    libmesh_assert(side.n_points() >= 3);

    // TODO: Fix Plane linking problem
//    Plane p = Plane(side.point(0), side.point(1), side.point(2));

//    d = (p0 - p.closest_point(p0)).size();
    d = 0.0;
    
  }

  return d;
}

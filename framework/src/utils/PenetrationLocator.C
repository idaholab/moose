#include "PenetrationLocator.h"

#include "boundary_info.h"
#include "elem.h"
//#include "plane.h"

PenetrationLocator::PenetrationLocator(Mesh & mesh, std::vector<unsigned int> master, unsigned int slave)
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

MeshBase::const_node_iterator nl = _mesh.local_nodes_begin();
  MeshBase::const_node_iterator end_nl = _mesh.local_nodes_end();
  for ( ; nl != end_nl ; ++nl)
  {
    const Node* node = *nl;

    std::vector< unsigned int >::iterator pos = std::find(node_list.begin(), node_list.end(), node->id());

    // see if this node is on a boundary and that boundary is the master boundary
    if (pos != node_list.end()
      //&& node_boundary_list[int(pos - node_list.begin())] == _master_boundary)
        && std::find(_master_boundary.begin(), _master_boundary.end(), node_boundary_list[int(pos - node_list.begin())]) != _master_boundary.end())
    {
      
      MeshBase::const_element_iterator el = _mesh.active_local_elements_begin();
      MeshBase::const_element_iterator end_el = _mesh.active_local_elements_end();
      for ( ; el != end_el ; ++el)
      {
        const Elem* elem = *el;

        std::vector< unsigned int >::iterator pos2 = std::find(elem_list.begin(), elem_list.end(), elem->id());

        // see if this elem is on a boundary and that boundary is the slave boundary
        // and that this elem contains the current node
        if (pos2 != elem_list.end()
            && id_list[int(pos2 - elem_list.begin())] == _slave_boundary
            && elem->contains_point(*node)) 
        {
            unsigned int side_num = *(side_list.begin() + int(pos2 - elem_list.begin()));

#ifdef DEBUG            
            std::cout << "Node " << node->id() << " contained in " << elem->id()
                      << " through side " << side_num
                      << ". Distance: " << normDistance(*(elem->build_side(side_num)), *node) << "\n";
#endif            

            _penetrated_elems[node->id()] = std::make_pair(elem->id(), normDistance(*(elem->build_side(side_num)), *node));
            
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

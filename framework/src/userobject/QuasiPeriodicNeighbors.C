/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "QuasiPeriodicNeighbors.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/mesh.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/elem.h"

template<>
InputParameters validParams<QuasiPeriodicNeighbors>()
{
  InputParameters params = validParams<GeneralUserObject>();
  //params.addParam<std::vector<std::string> >("auto_direction", "If using a generated mesh, you can specifiy just the dimension(s) you want to mark as periodic");
  params.addRequiredParam<unsigned int>("component", "The gradient component");
  return params;
}

QuasiPeriodicNeighbors::QuasiPeriodicNeighbors(const InputParameters & parameters) :
    GeneralUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _component(getParam<unsigned int>("component"))
{
}

void
QuasiPeriodicNeighbors::initialSetup()
{
  setQuasiPeriodicNeighbors();
}

void
QuasiPeriodicNeighbors::setQuasiPeriodicNeighbors()
{
  mooseWarning("Seetting quasiperiodic neighbors");

  std::list<std::pair<Elem *, std::pair<unsigned int, Elem *>>> neighbor_list;

  // pair the boundaries
  const std::pair<BoundaryID, BoundaryID> * boundary_ids = _mesh.getPairedBoundaryMapping(_component);

  // now store a translation vector
  RealVectorValue v;
  v(_component) = _mesh.dimensionWidth(_component);
  //Moose::out << "\n v = "; std::cout << v(0);

  // ok, we have now paired the boundaries along  the component
  // now we loop over the elements in the primary
  auto el  = _mesh.bndElemsBegin();
  auto end_el = _mesh.bndElemsEnd();
  for (; el != end_el ; ++el)
  {
    Elem * elem = (*el)->_elem;
    auto side = _mesh.sideWithBoundaryID(elem, boundary_ids->first);

    // only continue if we are on the primary boundary
    if (side != libMesh::invalid_uint)
    {
      // build element face that coincides with the boundary
      UniquePtr<Elem> current_side_elem = elem->build_side(side);

      // get the boundary face centroid
      const Point side_centroid = current_side_elem->centroid();

      //Moose::out << "\n side_centroid_x = "; std::cout << side_centroid(0);
      //translate the centroid to the opposing boundary
      const Point centroid_translated = side_centroid + v;
      //Moose::out << "\n side_centroid_translated_x = "; std::cout << centroid_translated(0);


      //use the point locator to find element on the opposite boundary
      const Elem * const_neighbor_elem = _mesh.getMesh().point_locator()(centroid_translated);

      //Moose::out << "\n neighbor elem id() = "; std::cout << const_neighbor_elem->id();

      // check to see if this point locator is null. It should not be!
      libmesh_assert(const_neighbor_elem);
      Elem * neighbor_elem = _mesh.getMesh().elem(const_neighbor_elem->id());

      // store the neighbor primary -> secondary relation
      neighbor_list.push_back({ elem, { side, neighbor_elem } });

      // store the neighbor secondary -> primary relation
      auto neighbor_side = _mesh.sideWithBoundaryID(neighbor_elem, boundary_ids->second);
      libmesh_assert(neighbor_side != libMesh::invalid_uint);
      neighbor_list.push_back({ neighbor_elem, { neighbor_side, elem } });
    }
  }
  // set all neighbors
  for (auto & n : neighbor_list)
    n.first->set_neighbor(n.second.first, n.second.second);
}

// QuasiPeriodicNeighbors::setQuasiPeriodicNeighbors()
//
// nl.dofMap()

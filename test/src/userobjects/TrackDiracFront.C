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

#include "TrackDiracFront.h"

template<>
InputParameters validParams<TrackDiracFront>()
{
  InputParameters params = validParams<NodalUserObject>();

  params.addRequiredCoupledVar("var", "Wherever this variable is close to 0.5 a Dirac point will be generated");

  return params;
}

TrackDiracFront::TrackDiracFront(const std::string & name, InputParameters parameters) :
    NodalUserObject(name, parameters),
    _var_value(coupledValue("var"))
{
}

void
TrackDiracFront::initialize()
{
  _dirac_points.clear();
}

void
TrackDiracFront::execute()
{
  // Is the value near 0.5?
  if(_var_value[_qp] > 0.4 && _var_value[_qp] < 0.6)
  {
    Elem * elem = localElementConnectedToCurrentNode();
    _dirac_points.push_back(std::make_pair(elem, *_current_node));
  }
}

void
TrackDiracFront::threadJoin(const UserObject &y)
{
  const TrackDiracFront & tdf = static_cast<const TrackDiracFront &>(y);

  // Merge in the values from "y"
  _dirac_points.insert(_dirac_points.end(), tdf._dirac_points.begin(), tdf._dirac_points.end());
}

void
TrackDiracFront::finalize()
{
  // Nothing to do because we were careful to only record _local_ information - so no MPI communication is necessary
}

Elem *
TrackDiracFront::localElementConnectedToCurrentNode()
{
  std::map< unsigned int, std::vector< unsigned int > > & _node_to_elem_map = _mesh.nodeToElemMap();

  dof_id_type id = _current_node->id();

  const std::vector< unsigned int > & connected_elems = _node_to_elem_map.at(id);

  unsigned int pid = libMesh::processor_id(); // This processor id

  // Look through all of the elements connected to this node and find one owned by the local processor
  for(unsigned int i=0; i<connected_elems.size(); i++)
  {
    Elem * elem = _mesh.elem(connected_elems[i]);

    if(elem->processor_id() == pid) // Is this element owned by the local processor?
      return elem;
  }

  mooseError("Unable to locate a local element connected to this node!");
}


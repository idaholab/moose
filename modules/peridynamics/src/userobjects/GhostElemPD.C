//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostElemPD.h"
#include "MeshBasePD.h"

registerMooseObject("PeridynamicsApp", GhostElemPD);

template <>
InputParameters
validParams<GhostElemPD>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addClassDescription("Class for ghosting elements accross processors");

  return params;
}

GhostElemPD::GhostElemPD(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _pdmesh(dynamic_cast<MeshBasePD &>(_mesh))
{
  ghostElements();
}

void
GhostElemPD::meshChanged()
{
  ghostElements();
}

void
GhostElemPD::ghostElements()
{
  // scheme 1: loop through only the local elements and ghost only the nonlocal ones connected to
  // current processor this is currently NOT working for one tests:
  // plane_strain_prescribed_ordinary_state.i
  const MeshBase::const_element_iterator end_elem = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator elem = _mesh.getMesh().active_local_elements_begin();
       elem != end_elem;
       ++elem)
    for (unsigned int i = 0; i < 2; ++i)
      for (unsigned int j = 0; j < _pdmesh.nneighbors((*elem)->get_node(i)->id()); ++j)
        _subproblem.addGhostedElem(_pdmesh.bonds((*elem)->get_node(i)->id())[j]);

  //  // scheme 2: loop through only the local nodes and ghost their neighbors and their neighbors' neighbor to current processor
  //  //  this is currently NOT working !!!
  // const MeshBase::const_node_iterator end_node = _mesh.getMesh().local_nodes_end();
  // for (MeshBase::const_node_iterator node = _mesh.getMesh().local_nodes_begin(); node !=
  // end_node; ++node)
  //   for (unsigned int i = 0; i < _pdmesh.nneighbors((*node)->id()); ++i)
  //   {
  //     dof_id_type nb_i = _pdmesh.neighbors((*node)->id())[i];
  //     if (_mesh.nodePtr(nb_i)->processor_id() != processor_id())
  //       for (unsigned int j = 0; j < _pdmesh.nneighbors(nb_i); ++j)
  //         _subproblem.addGhostedElem(_pdmesh.bonds(nb_i)[j]);
  //   }

  // // scheme 3: loop through all the nodes and ghost all the nonlocal ones
  // // this will ghost all the elements
  // const MeshBase::const_node_iterator end_node = _mesh.getMesh().nodes_end();
  // for (MeshBase::const_node_iterator node = _mesh.getMesh().nodes_begin(); node != end_node;
  // ++node)
  //   for (unsigned int i = 0; i < _pdmesh.nneighbors((*node)->id()); ++i)
  //     _subproblem.addGhostedElem(_pdmesh.bonds((*node)->id())[i]);
}

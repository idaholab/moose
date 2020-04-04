//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GhostElemPD.h"
#include "PeridynamicsMesh.h"

registerMooseObject("PeridynamicsApp", GhostElemPD);

InputParameters
GhostElemPD::validParams()
{
  InputParameters params = GeneralUserObjectBasePD::validParams();
  params.addClassDescription("Class for ghosting elements accross processors");

  return params;
}

GhostElemPD::GhostElemPD(const InputParameters & parameters) : GeneralUserObjectBasePD(parameters)
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
  // Loop through the active local elements and ghost elements from other processors due to
  // formulation nonlocality
  const MeshBase::const_element_iterator end_elem = _mesh.getMesh().active_local_elements_end();
  for (MeshBase::const_element_iterator elem = _mesh.getMesh().active_local_elements_begin();
       elem != end_elem;
       ++elem)
    if ((*elem)->type() == 0) // only ghost neighbors for Edge2 elems
      for (unsigned int i = 0; i < _nnodes; ++i)
      {
        std::vector<dof_id_type> neighbors = _pdmesh.getNeighbors((*elem)->node_id(i));
        for (unsigned int j = 0; j < neighbors.size(); ++j)
          _subproblem.addGhostedElem(_pdmesh.getBonds((*elem)->node_id(i))[j]);
      }
}

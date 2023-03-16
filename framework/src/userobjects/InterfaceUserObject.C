//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUserObject.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Assembly.h"

#include "libmesh/remote_elem.h"

InputParameters
InterfaceUserObject::validParams()
{
  InputParameters params = InterfaceUserObjectBase::validParams();
  params.addClassDescription("Basic UO class to perform computation across an interface");
  return params;
}

InterfaceUserObject::InterfaceUserObject(const InputParameters & parameters)
  : InterfaceUserObjectBase(parameters), _has_fv_vars(false), _fi(nullptr)
{
  // Check for finite volume variables
  // const std::vector<MooseVariableFEBase *> & coupled_vars = getCoupledMooseVars();
  for (const auto & var : _coupled_moose_vars)
    if (var->isFV())
      _has_fv_vars = true;
}

void
InterfaceUserObject::execute()
{
  if (_has_fv_vars)
  {
    // Retrieve the face info from the mesh
    _fi = _mesh.faceInfo(_current_elem, _current_side);
    if (!_fi)
    {
      // Let's check the other side
      const Elem * const neighbor = _current_elem->neighbor_ptr(_current_side);
      mooseAssert(neighbor != remote_elem,
                  "I'm pretty confident that if we got here then our neighbor should be "
                  "local/ghosted/null");
      if (neighbor)
      {
        const auto neigh_side = neighbor->which_neighbor_am_i(_current_elem);
        _fi = _mesh.faceInfo(neighbor, neigh_side);
      }

      if (!_fi)
        // We still don't have a face info. It must be owned by another process
        return;
    }

    auto pr = _face_infos_processed.insert(_fi);
    if (!pr.second)
      // Insertion didn't happen so we must have already processed this FaceInfo
      return;
  }
}

void
InterfaceUserObject::initialize()
{
  if (_has_fv_vars)
    _face_infos_processed.clear();
}

const Real &
InterfaceUserObject::getNeighborElemVolume()
{
  return _current_neighbor_volume;
}

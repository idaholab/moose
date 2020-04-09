//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XFEMMaterialStateMarkerBase.h"

#include "XFEM.h"
#include "MooseMesh.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"

InputParameters
XFEMMaterialStateMarkerBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addParam<std::vector<BoundaryName>>(
      "initiate_on_boundary",
      "Permit cracks to initiate in elements adjacent to specified boundaries");
  params.addParam<bool>("secondary_cracks", false, "should secondary cracks be allowed");
  return params;
}

XFEMMaterialStateMarkerBase::XFEMMaterialStateMarkerBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _secondary_cracks(getParam<bool>("secondary_cracks"))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in XFEMMaterialStateMarkerBase");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in XFEMMaterialStateMarkerBase");
  if (isNodal())
    mooseError("XFEMMaterialStateMarkerBase can only be run on an element variable");

  if (isParamValid("initiate_on_boundary"))
  {
    std::vector<BoundaryName> initiation_boundary_names =
        getParam<std::vector<BoundaryName>>("initiate_on_boundary");
    _initiation_boundary_ids = _mesh.getBoundaryIDs(initiation_boundary_names, true);
  }
}

void
XFEMMaterialStateMarkerBase::initialize()
{
  _marked_elems.clear();
  _marked_frags
      .clear(); // mark the fragment which has secondary crack growing from the primary crack
  _marked_elem_sides.clear();
}

void
XFEMMaterialStateMarkerBase::execute()
{
  RealVectorValue direction;
  bool isCut = _xfem->isElemCut(_current_elem);
  bool isCTE = _xfem->isElemAtCrackTip(_current_elem);
  bool isOnBoundary = false;
  unsigned int boundarySide = 99999;
  unsigned int _current_eid = _current_elem->id();
  std::map<unsigned int, RealVectorValue>::iterator mit;
  mit = _marked_elems.find(_current_eid);

  for (unsigned int i = 0; i < _initiation_boundary_ids.size(); ++i)
  {
    if (_mesh.isBoundaryElem(_current_eid, _initiation_boundary_ids[i]))
    {
      isOnBoundary = true;
      boundarySide = _mesh.sideWithBoundaryID(_current_elem, _initiation_boundary_ids[i]);
    }
  }

  if (isCTE && doesElementCrack(direction))
  {
    if (mit != _marked_elems.end())
    {
      mooseError("ERROR: element ", _current_eid, " already marked for crack growth.");
    }
    _marked_elems[_current_eid] = direction;
  }
  else if (isOnBoundary && doesElementCrack(direction))
  {
    if (mit != _marked_elems.end())
    {
      mooseError("ERROR: element ", _current_eid, " already marked for crack growth.");
    }
    _marked_elems[_current_eid] = direction;
    _marked_elem_sides[_current_eid] = boundarySide;
  }
  else if (isCut && _secondary_cracks && doesElementCrack(direction))
  {
    if (mit != _marked_elems.end())
    {
      mooseError("ERROR: element ", _current_eid, " already marked for crack growth.");
    }
    _marked_elems[_current_eid] = direction;
    _marked_frags.insert(_current_eid);
  }
}

void
XFEMMaterialStateMarkerBase::threadJoin(const UserObject & y)
{
  const XFEMMaterialStateMarkerBase & xmuo = dynamic_cast<const XFEMMaterialStateMarkerBase &>(y);

  for (std::map<unsigned int, RealVectorValue>::const_iterator mit = xmuo._marked_elems.begin();
       mit != xmuo._marked_elems.end();
       ++mit)
  {
    _marked_elems[mit->first] = mit->second; // TODO do error checking for duplicates here too
  }

  for (std::set<unsigned int>::const_iterator mit = xmuo._marked_frags.begin();
       mit != xmuo._marked_frags.end();
       ++mit)
  {
    _marked_frags.insert(*mit); // TODO do error checking for duplicates here too
  }

  for (std::map<unsigned int, unsigned int>::const_iterator mit = xmuo._marked_elem_sides.begin();
       mit != xmuo._marked_elem_sides.end();
       ++mit)
  {
    _marked_elem_sides[mit->first] = mit->second; // TODO do error checking for duplicates here too
  }
}

void
XFEMMaterialStateMarkerBase::finalize()
{
  _communicator.set_union(_marked_elems);
  _communicator.set_union(_marked_frags);
  _communicator.set_union(_marked_elem_sides);

  _xfem->clearStateMarkedElems();
  std::map<unsigned int, RealVectorValue>::iterator mit;
  for (mit = _marked_elems.begin(); mit != _marked_elems.end(); ++mit)
  {
    if (_marked_elem_sides.find(mit->first) != _marked_elem_sides.end())
    {
      _xfem->addStateMarkedElem(mit->first, mit->second, _marked_elem_sides[mit->first]);
    }
    else if (_marked_frags.find(mit->first) != _marked_frags.end())
    {
      _xfem->addStateMarkedFrag(mit->first, mit->second);
    }
    else
    {
      _xfem->addStateMarkedElem(mit->first, mit->second);
    }
  }
  _marked_elems.clear();
  _marked_frags.clear();
  _marked_elem_sides.clear();
}

bool
XFEMMaterialStateMarkerBase::doesElementCrack(RealVectorValue & direction)
{
  direction(1) = 1.0;
  return true;
}

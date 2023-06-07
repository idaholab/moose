//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCutNucleationBase.h"

#include "MooseMesh.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"

InputParameters
MeshCutNucleationBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "initiate_on_boundary",
      "Cracks can only initiate on elements adjacent to specified boundaries.");
  // Make this userobject execute before every other standard UO including meshCutterUO
  params.setDocString(
      "execution_order_group",
      "Nucleation UO needs to be completely executed before GeometricCutUserObject.");
  params.set<int>("execution_order_group") = -1;

  // This needs to have the same execute_on flag as the GeometricCutUserObject
  // fixme lynn maybe hte execute_on flag should be suppressed in both of them
  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString()); // fixme is this necessary
  params.set<ExecFlagEnum>("execute_on") = EXEC_XFEM_MARK;
  // params.registerBase("MeshCutNucleationBase"); // fixme do I need this?
  return params;
}

MeshCutNucleationBase::MeshCutNucleationBase(const InputParameters & parameters)
  : ElementUserObject(parameters), _mesh(_subproblem.mesh())
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    mooseError("Problem casting _subproblem to FEProblemBase in MeshCutNucleationBase");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == nullptr)
    mooseError("Problem casting to XFEM in MeshCutNucleationBase");
  if (isNodal())
    mooseError("MeshCutNucleationBase can only be run on an element variable");

  if (isParamValid("initiate_on_boundary"))
  {
    std::vector<BoundaryName> initiation_boundary_names =
        getParam<std::vector<BoundaryName>>("initiate_on_boundary");
    _initiation_boundary_ids = _mesh.getBoundaryIDs(initiation_boundary_names, true);
  }
}

void
MeshCutNucleationBase::initialize()
{
  _nucleated_elems.clear();
}

void
MeshCutNucleationBase::execute()
{
  std::pair<RealVectorValue, RealVectorValue> cutterElemNodes;
  bool isCut = _xfem->isElemCut(_current_elem);

  bool isOnBoundary = false;
  unsigned int boundarySide = std::numeric_limits<unsigned int>::max();
  unsigned int current_eid = _current_elem->id();
  std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>>::iterator mit;
  mit = _nucleated_elems.find(current_eid);

  for (unsigned int i = 0; i < _initiation_boundary_ids.size(); ++i)
  {
    if (_mesh.isBoundaryElem(current_eid, _initiation_boundary_ids[i]))
    {
      isOnBoundary = true;
      boundarySide = _mesh.sideWithBoundaryID(_current_elem, _initiation_boundary_ids[i]);
    }
  }
  // fixme lynn, only allowing one nucleation per element
  if (!isCut && isOnBoundary && doesElementCrack(cutterElemNodes))
  {
    if (mit != _nucleated_elems.end())
    {
      mooseError("ERROR: element ", current_eid, " already marked for crack growth.");
    }
    _nucleated_elems[current_eid] = cutterElemNodes;
  }
}

void
MeshCutNucleationBase::threadJoin(const UserObject & y)
{
  const MeshCutNucleationBase & xmuo = dynamic_cast<const MeshCutNucleationBase &>(y);

  for (std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>>::const_iterator mit =
           xmuo._nucleated_elems.begin();
       mit != xmuo._nucleated_elems.end();
       ++mit)
  {
    _nucleated_elems[mit->first] = mit->second; // TODO do error checking for duplicates here too
  }
}

void
MeshCutNucleationBase::finalize()
{
  _communicator.set_union(_nucleated_elems);
  // fixme, I can't clear these because I want them to be available to the mesh cutter
  //  _nucleated_elems.clear();
}

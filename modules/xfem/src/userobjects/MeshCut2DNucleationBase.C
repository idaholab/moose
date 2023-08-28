//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCut2DNucleationBase.h"
#include "XFEMAppTypes.h"
#include "XFEM.h"

#include "MooseMesh.h"

#include "libmesh/parallel_algebra.h"
#include "libmesh/parallel.h"

InputParameters
MeshCut2DNucleationBase::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<std::vector<BoundaryName>>(
      "initiate_on_boundary",
      "Cracks can only initiate on elements adjacent to specified boundaries.");
  params.addParam<Real>("nucleation_radius",
                        0,
                        "Cracks will only nucleate if they are outside the nucleation_radius of an "
                        "existing crack.");
  // Make this userobject execute before every other standard UO including meshCutterUO
  params.setDocString(
      "execution_order_group",
      "Nucleation UO needs to be completely executed before GeometricCutUserObject.");
  params.set<int>("execution_order_group") = -1;

  ExecFlagEnum & exec = params.set<ExecFlagEnum>("execute_on");
  exec.addAvailableFlags(EXEC_XFEM_MARK);
  params.setDocString("execute_on", exec.getDocString());
  params.set<ExecFlagEnum>("execute_on") = EXEC_XFEM_MARK;
  return params;
}

MeshCut2DNucleationBase::MeshCut2DNucleationBase(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_subproblem.mesh()),
    _nucleation_radius(getParam<Real>("nucleation_radius"))
{
  FEProblemBase * fe_problem = dynamic_cast<FEProblemBase *>(&_subproblem);
  if (fe_problem == NULL)
    paramError("Problem casting _subproblem to FEProblemBase in MeshCut2DNucleationBase");
  _xfem = MooseSharedNamespace::dynamic_pointer_cast<XFEM>(fe_problem->getXFEM());
  if (_xfem == nullptr)
    paramError("Problem casting to XFEM in MeshCut2DNucleationBase");

  if (isParamValid("initiate_on_boundary"))
  {
    std::vector<BoundaryName> initiation_boundary_names =
        getParam<std::vector<BoundaryName>>("initiate_on_boundary");
    _initiation_boundary_ids = _mesh.getBoundaryIDs(initiation_boundary_names, true);
  }
}

void
MeshCut2DNucleationBase::initialize()
{
  _nucleated_elems.clear();
}

void
MeshCut2DNucleationBase::execute()
{
  std::pair<RealVectorValue, RealVectorValue> cutterElemNodes;
  bool is_cut = _xfem->isElemCut(_current_elem);
  if (_current_elem->processor_id() != processor_id())
    return;

  bool isOnBoundary = false;
  unsigned int current_eid = _current_elem->id();
  std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>>::iterator mit;
  mit = _nucleated_elems.find(current_eid);

  for (unsigned int i = 0; i < _initiation_boundary_ids.size(); ++i)
    if (_mesh.isBoundaryElem(current_eid, _initiation_boundary_ids[i]))
      isOnBoundary = true;
  // This does not currently allow for nucleation in an element that is already cut
  if (!is_cut && isOnBoundary && doesElementCrack(cutterElemNodes))
  {
    if (mit != _nucleated_elems.end())
    {
      mooseError("ERROR: element ", current_eid, " already marked for crack nucleation.");
    }
    _nucleated_elems[current_eid] = cutterElemNodes;
  }
}

void
MeshCut2DNucleationBase::threadJoin(const UserObject & y)
{
  const MeshCut2DNucleationBase & xmuo = dynamic_cast<const MeshCut2DNucleationBase &>(y);

  for (std::map<unsigned int, std::pair<RealVectorValue, RealVectorValue>>::const_iterator mit =
           xmuo._nucleated_elems.begin();
       mit != xmuo._nucleated_elems.end();
       ++mit)
  {
    _nucleated_elems[mit->first] = mit->second; // TODO do error checking for duplicates here too
  }
}

void
MeshCut2DNucleationBase::finalize()
{
  _communicator.set_union(_nucleated_elems);
  // _nucleated_elems is not cleared here because it needs to be available to the mesh cutter
}

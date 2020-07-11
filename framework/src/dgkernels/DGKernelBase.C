//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DGKernelBase.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

defineLegacyParams(DGKernelBase);

InputParameters
DGKernelBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params += TwoMaterialPropertyInterface::validParams();
  params += TransientInterface::validParams();
  params += BlockRestrictable::validParams();
  params += BoundaryRestrictable::validParams();
  params += MeshChangedInterface::validParams();
  params += TaggingInterface::validParams();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addPrivateParam<bool>("_use_undisplaced_reference_points", false);
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  params.addParam<std::vector<AuxVariableName>>(
      "save_in",
      "The name of auxiliary variables to save this Kernel's residual contributions to. "
      " Everything about that variable must match everything about this variable (the "
      "type, what blocks it's on, etc.)");
  params.addParam<std::vector<AuxVariableName>>(
      "diag_save_in",
      "The name of auxiliary variables to save this Kernel's diagonal Jacobian "
      "contributions to. Everything about that variable must match everything "
      "about this variable (the type, what blocks it's on, etc.)");

  // DG Kernels always need one layer of ghosting.
  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);

  params.addParam<std::vector<BoundaryName>>(
      "exclude_boundary", "The internal side sets to be excluded from this kernel.");
  params.registerBase("DGKernel");

  return params;
}

// Static mutex definitions
Threads::spin_mutex DGKernelBase::_resid_vars_mutex;
Threads::spin_mutex DGKernelBase::_jacoby_vars_mutex;

DGKernelBase::DGKernelBase(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(this),
    BoundaryRestrictable(this, false), // false for _not_ nodal
    SetupInterface(this),
    TransientInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    TwoMaterialPropertyInterface(this, blockIDs(), boundaryIDs()),
    Restartable(this, "DGKernels"),
    MeshChangedInterface(parameters),
    TaggingInterface(this),
    ElementIDInterface(this),
    _subproblem(*getCheckedPointerParam<SubProblem *>("_subproblem")),
    _sys(*getCheckedPointerParam<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _mesh(_subproblem.mesh()),

    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),

    _neighbor_elem(_assembly.neighbor()),
    _neighbor_elem_volume(_assembly.neighborVolume()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),

    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),
    _normals(_assembly.normals()),

    _save_in_strings(parameters.get<std::vector<AuxVariableName>>("save_in")),
    _diag_save_in_strings(parameters.get<std::vector<AuxVariableName>>("diag_save_in"))
{
  // Gather information on broken boundaries
  std::vector<BoundaryName> bnd = isParamValid("exclude_boundary")
                                      ? getParam<std::vector<BoundaryName>>("exclude_boundary")
                                      : std::vector<BoundaryName>(0);
  auto bnd_ids = _mesh.getBoundaryIDs(bnd);

  // check if the broken boundary ids are valid
  auto & valid_ids = _mesh.meshSidesetIds();
  std::vector<BoundaryName> diff;
  for (unsigned int i = 0; i < bnd_ids.size(); ++i)
    if (valid_ids.find(bnd_ids[i]) == valid_ids.end())
      diff.push_back(bnd[i]);
  if (!diff.empty())
  {
    auto msg = "DGKernel '" + name() +
               "' contains the following boundary names that do not exist on the mesh: " +
               Moose::stringify(diff, ", ");
    paramError("exclude_boundary", msg);
  }

  _excluded_boundaries.insert(bnd_ids.begin(), bnd_ids.end());
}

DGKernelBase::~DGKernelBase() {}

void
DGKernelBase::computeResidual()
{
  if (!excludeBoundary())
  {
    // Compute the residual for this element
    computeElemNeighResidual(Moose::Element);

    // Compute the residual for the neighbor
    computeElemNeighResidual(Moose::Neighbor);
  }
}

void
DGKernelBase::computeJacobian()
{
  if (!excludeBoundary())
  {
    // Compute element-element Jacobian
    computeElemNeighJacobian(Moose::ElementElement);

    // Compute element-neighbor Jacobian
    computeElemNeighJacobian(Moose::ElementNeighbor);

    // Compute neighbor-element Jacobian
    computeElemNeighJacobian(Moose::NeighborElement);

    // Compute neighbor-neighbor Jacobian
    computeElemNeighJacobian(Moose::NeighborNeighbor);
  }
}

void
DGKernelBase::computeOffDiagJacobian(unsigned int jvar)
{
  if (!excludeBoundary())
  {
    if (jvar == variable().number())
      computeJacobian();
    else
    {
      // Compute element-element Jacobian
      computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);

      // Compute element-neighbor Jacobian
      computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);

      // Compute neighbor-element Jacobian
      computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);

      // Compute neighbor-neighbor Jacobian
      computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
    }
  }
}

bool
DGKernelBase::excludeBoundary() const
{
  if (_excluded_boundaries.empty())
    return false;

  auto boundary_ids = _mesh.getBoundaryIDs(_current_elem, _current_side);
  for (auto bid : boundary_ids)
    if (_excluded_boundaries.find(bid) != _excluded_boundaries.end())
      return true;

  // make sure we will also break on the neighboring side
  unsigned int neighbor_side = _neighbor_elem->which_neighbor_am_i(_current_elem);
  boundary_ids = _mesh.getBoundaryIDs(_neighbor_elem, neighbor_side);
  for (auto bid : boundary_ids)
    if (_excluded_boundaries.find(bid) != _excluded_boundaries.end())
      return true;

  return false;
}

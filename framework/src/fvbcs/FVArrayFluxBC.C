//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVArrayFluxBC.h"
#include "MooseVariableFV.h"
#include "SystemBase.h"
#include "Assembly.h"
#include "ADUtils.h"

InputParameters
FVArrayFluxBC::validParams()
{
  InputParameters params = FVFluxBCBase::validParams();
  return params;
}

FVArrayFluxBC::FVArrayFluxBC(const InputParameters & parameters)
  : FVFluxBCBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            false,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariableFV()),
    _u(_var.adSln())
{
  addMooseVariableDependency(&_var);
}

void
FVArrayFluxBC::computeResidual(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a residual
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  auto r = MetaPhysicL::raw_value(fi.faceArea() * fi.faceCoord() * computeQpResidual());

  // This could be an "internal" boundary - one created by variable block
  // restriction where the var is only defined on one side of the face.  We
  // need to make sure that we add the residual contribution to the correct
  // side - the one where the variable is defined.
  if (ft == FaceInfo::VarFaceNeighbors::ELEM)
    prepareVectorTag(_assembly, _var.number());
  else if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    prepareVectorTagNeighbor(_assembly, _var.number());
  else
    mooseError("should never get here");

  _assembly.saveLocalArrayResidual(_local_re, 0, 1, r);
  accumulateTaggedLocalResidual();
}

void
FVArrayFluxBC::computeJacobian(const FaceInfo & fi)
{
  _face_info = &fi;
  _normal = fi.normal();
  auto ft = fi.faceType(_var.name());

  // For FV flux kernels, the normal is always oriented outward from the lower-id
  // element's perspective.  But for BCs, there is only a Jacobian
  // contribution to one element (one side of the face).  Because of this, we
  // make an exception and orient the normal to point outward from whichever
  // side of the face the BC's variable is defined on; we flip it if this
  // variable is defined on the neighbor side of the face (instead of elem) since
  // the FaceInfo normal polarity is always oriented with respect to the lower-id element.
  if (ft == FaceInfo::VarFaceNeighbors::NEIGHBOR)
    _normal = -_normal;

  ADRealEigenVector r = fi.faceArea() * fi.faceCoord() * computeQpResidual();

  mooseAssert(_var.dofIndices().size() == 1, "We're currently built to use CONSTANT MONOMIALS");

  auto local_functor = [&](const ADReal &, dof_id_type, const std::set<TagID> &) {
    mooseError("FV Array variables do not support non-global AD DOF indexing");
  };

  for (unsigned int v = 0; v < _var.count(); v++)
  {
    auto dof = _var.dofIndices()[0];
    const unsigned int ndofs = 1;
    _assembly.processDerivatives(
        r(v), Moose::globalADArrayOffset(dof, ndofs), _matrix_tags, local_functor);
  }
}

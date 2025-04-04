//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSFVVariable.h"
#include "InputParameters.h"
#include "MooseError.h"
#include "INSFVFullyDevelopedFlowBC.h"
#include "Attributes.h"
#include "SubProblem.h"
#include "MooseApp.h"
#include "TheWarehouse.h"
#include "INSFVAttributes.h"
#include "SystemBase.h"
#include "INSFVHydraulicSeparatorInterface.h"

InputParameters
INSFVVariable::validParams()
{
  return MooseVariableFVReal::validParams();
}

INSFVVariable::INSFVVariable(const InputParameters & params)
  : MooseVariableFVReal(params), _qp_calculations(false)
{
}

void
INSFVVariable::initialSetup()
{
  MooseVariableFVReal::initialSetup();

  _boundary_id_to_separator.clear();
  std::vector<FVFluxBC *> bcs;

  const auto base_query = this->_subproblem.getMooseApp()
                              .theWarehouse()
                              .query()
                              .template condition<AttribSystem>("FVFluxBC")
                              .template condition<AttribThread>(_tid);

  for (const auto bnd_id : this->_mesh.getBoundaryIDs())
  {
    auto base_query_copy = base_query;
    base_query_copy.template condition<AttribBoundaries>(std::set<BoundaryID>({bnd_id}))
        .queryInto(bcs);
    for (const auto bc : bcs)
    {
      const auto separator = dynamic_cast<const INSFVHydraulicSeparatorInterface *>(bc);
      if (separator)
        _boundary_id_to_separator.emplace(bnd_id, bc);
    }
  }
}

bool
INSFVVariable::isSeparatorBoundary(const FaceInfo & fi) const
{
  for (const auto bid : fi.boundaryIDs())
    if (_boundary_id_to_separator.find(bid) != _boundary_id_to_separator.end())
      return true;

  return false;
}

bool
INSFVVariable::isExtrapolatedBoundaryFace(const FaceInfo & fi,
                                          const Elem * const elem,
                                          const Moose::StateArg & time) const
{
  if (isDirichletBoundaryFace(fi, elem, time))
    return false;
  if (!isInternalFace(fi))
    // We are neither a Dirichlet nor an internal face
    return true;

  // If we got here, then we're definitely on an internal face
  if (isSeparatorBoundary(fi))
    return true;

  return false;
}

bool
INSFVVariable::isFullyDevelopedFlowFace(const FaceInfo & fi) const
{
  const auto & face_type = fi.faceType(std::make_pair(this->number(), this->sys().number()));

  mooseAssert(face_type != FaceInfo::VarFaceNeighbors::NEITHER,
              "I'm concerned that if you're calling this method with a FaceInfo that doesn't have "
              "this variable defined on either side, that you are doing something dangerous.");

  // If we're defined on both sides of the face, then we're not a boundary face
  if (face_type == FaceInfo::VarFaceNeighbors::BOTH)
    return false;

  std::vector<INSFVFullyDevelopedFlowBC *> bcs;

  this->_subproblem.getMooseApp()
      .theWarehouse()
      .query()
      .template condition<AttribINSFVBCs>(INSFVBCs::INSFVFullyDevelopedFlowBC)
      .template condition<AttribBoundaries>(fi.boundaryIDs())
      .queryInto(bcs);

  return !bcs.empty();
}

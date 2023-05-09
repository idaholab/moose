//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

InputParameters
INSFVVariable::validParams()
{
  return MooseVariableFVReal::validParams();
}

INSFVVariable::INSFVVariable(const InputParameters & params)
  : MooseVariableFVReal(params), _qp_calculations(false)
{
}

bool
INSFVVariable::isFullyDevelopedFlowFace(const FaceInfo & fi) const
{
  const auto & face_type = fi.faceType(this->name());

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

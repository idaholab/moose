//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GetAllTheRCVelocities.h"
#include "INSFVRhieChowInterpolator.h"
#include "SubProblem.h"
#include "MooseMesh.h"

registerMooseObject("NavierStokesTestApp", GetAllTheRCVelocities);

InputParameters
GetAllTheRCVelocities::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRelationshipManager("GhostEverything",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  params.addParam<UserObjectName>("rc_uo", "The Rhie-Chow user object");
  return params;
}

GetAllTheRCVelocities::GetAllTheRCVelocities(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _rc_uo(
        const_cast<INSFVRhieChowInterpolator &>(getUserObject<INSFVRhieChowInterpolator>("rc_uo")))
{
  _rc_uo.pullAllNonlocal();
}

void
GetAllTheRCVelocities::execute()
{
  for (const auto & fi : _subproblem.mesh().allFaceInfo())
    _rc_uo.getVelocity(Moose::FV::InterpMethod::RhieChow, fi, Moose::currentState(), 0);
}

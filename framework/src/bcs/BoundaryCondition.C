//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BoundaryCondition.h"

InputParameters
BoundaryCondition::validParams()
{
  InputParameters params = ResidualObject::validParams();
  params += BoundaryRestrictableRequired::validParams();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");

  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");
  params.addCoupledVar("displacements", "The displacements");
  params.registerBase("BoundaryCondition");

  return params;
}

BoundaryCondition::BoundaryCondition(const InputParameters & parameters, bool nodal)
  : ResidualObject(parameters, nodal),
    BoundaryRestrictableRequired(this, nodal),
    DistributionInterface(this),
    GeometricSearchInterface(this)
{
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AttachDummySystemConstraint.h"

#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseTestApp", AttachDummySystemConstraint);

InputParameters
AttachDummySystemConstraint::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Attaches a libMesh constraint object that adds no constraints to the "
                             "first nonlinear system.");
  return params;
}

AttachDummySystemConstraint::AttachDummySystemConstraint(const InputParameters & params)
  : GeneralUserObject(params)
{
  _fe_problem.getNonlinearSystemBase(0).system().attach_constraint_object(_dummy_constraint);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MFEMScalarIC.h"
#include "MFEMProblem.h"
#include <libmesh/libmesh_common.h>
#include <mfem.hpp>

registerMooseObject("MooseApp", MFEMScalarIC);

InputParameters
MFEMScalarIC::validParams()
{
  auto params = MFEMGeneralUserObject::validParams();
  params.addClassDescription("Sets the initial values of an MFEM scalar variable from a "
                             "user-specified scalar coefficient.");
  params.addRequiredParam<std::string>("variable",
                                       "The variable to apply the initial condition for");
  params.addRequiredParam<MFEMScalarCoefficientName>("coefficient", "The scalar coefficient");
  params.registerBase("InitialCondition");
  // We cannot generally execute this at construction time since the coefficient may be based on a
  // MOOSE function which is not itself setup until its initialSetup is called. UserObject initial
  // execution occurs after function initialSetup
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

MFEMScalarIC::MFEMScalarIC(const InputParameters & params) : MFEMGeneralUserObject(params) {}

void
MFEMScalarIC::execute()
{
  auto & coeff = getScalarCoefficient(getParam<MFEMScalarCoefficientName>("coefficient"));
  auto grid_function = getMFEMProblem().getGridFunction(getParam<std::string>("variable"));
  grid_function->ProjectCoefficient(coeff);
}

#endif

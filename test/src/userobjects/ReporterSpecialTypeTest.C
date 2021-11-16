//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterSpecialTypeTest.h"

registerMooseObject("MooseTestApp", ReporterSpecialTypeTest);

InputParameters
ReporterSpecialTypeTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<ReporterName>(
      "pp_reporter", "Test parameter that should represent a Postprocessor value");
  params.addRequiredParam<ReporterName>(
      "vpp_reporter", "Test parameter that should represent a VectorPostprocessor value");

  return params;
}

ReporterSpecialTypeTest::ReporterSpecialTypeTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  getReporterValue<Real>("pp_reporter");
  getReporterValue<VectorPostprocessorValue>("vpp_reporter");

  for (const auto & name : {"pp_reporter", "vpp_reporter"})
    if (isPostprocessor(name) || isVectorPostprocessor(name))
      mooseError("Is a special type");
}

void
ReporterSpecialTypeTest::initialSetup()
{
  if (!hasUserObjectByName<Postprocessor>(getReporterName("pp_reporter").getObjectName()))
    mooseError("Is not a pp");
  if (!hasUserObjectByName<VectorPostprocessor>(getReporterName("vpp_reporter").getObjectName()))
    mooseError("Is not a vpp");
  if (!isPostprocessor("pp_reporter"))
    mooseError("Not a pp special type");
  if (!isVectorPostprocessor("vpp_reporter"))
    mooseError("Not a vpp special type");
}

bool
ReporterSpecialTypeTest::isPostprocessor(const std::string & param_name) const
{
  return _fe_problem.getReporterData()
      .getReporterStateBase(getReporterName(param_name))
      .getReporterName()
      .isPostprocessor();
}

bool
ReporterSpecialTypeTest::isVectorPostprocessor(const std::string & param_name) const
{
  return _fe_problem.getReporterData()
      .getReporterStateBase(getReporterName(param_name))
      .getReporterName()
      .isVectorPostprocessor();
}

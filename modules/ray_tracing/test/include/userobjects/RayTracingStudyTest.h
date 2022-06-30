//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RepeatableRayStudy.h"

class RayTracingStudyTest : public RayTracingStudy
{
public:
  RayTracingStudyTest(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void generateRays() override;
  void postExecuteStudy() override;
};

class RayTracingStudyNoBankingTest : public RayTracingStudy
{
public:
  RayTracingStudyNoBankingTest(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void generateRays() override {}
};

class RayTracingStudyWithRegistrationTest : public RayTracingStudy
{
public:
  RayTracingStudyWithRegistrationTest(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  void generateRays() override {}
};

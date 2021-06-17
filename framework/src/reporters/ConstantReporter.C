//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantReporter.h"

registerMooseObject("MooseApp", ConstantReporter);

InputParameters
ConstantReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<int>("integer");
  params += addReporterTypeParams<Real>("real");
  params += addReporterTypeParams<std::string>("string");

  return params;
}

ConstantReporter::ConstantReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareConstantReporterValues<int>("integer")),
    _real(declareConstantReporterValues<Real>("real")),
    _string(declareConstantReporterValues<std::string>("string")),
    _int_vec(declareConstantVectorReporterValues<int>("integer")),
    _real_vec(declareConstantVectorReporterValues<Real>("real")),
    _string_vec(declareConstantVectorReporterValues<std::string>("string"))
{
}

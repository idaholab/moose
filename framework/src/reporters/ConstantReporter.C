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
  params += addReporterTypeParams<dof_id_type>("dof_id_type");
  params += addReporterTypeParams<Point>("point");

  return params;
}

ConstantReporter::ConstantReporter(const InputParameters & parameters) : GeneralReporter(parameters)
{
  declareConstantReporterValues<int>("integer");
  declareConstantReporterValues<Real>("real");
  declareConstantReporterValues<std::string>("string");
  declareConstantReporterValues<dof_id_type>("dof_id_type");
  declareConstantReporterValues<Point>("point");
  declareConstantVectorReporterValues<int>("integer");
  declareConstantVectorReporterValues<Real>("real");
  declareConstantVectorReporterValues<std::string>("string");
  declareConstantVectorReporterValues<dof_id_type>("dof_id_type");
}

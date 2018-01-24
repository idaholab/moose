//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Elastic.h"

template <>
InputParameters
validParams<Elastic>()
{
  InputParameters params = validParams<SolidModel>();
  params.addClassDescription("A simple hypo-elastic model");
  return params;
}

Elastic::Elastic(const InputParameters & parameters) : SolidModel(parameters)
{

  createConstitutiveModel("ElasticModel");
}

////////////////////////////////////////////////////////////////////////

Elastic::~Elastic() {}

////////////////////////////////////////////////////////////////////////

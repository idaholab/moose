/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Elastic.h"

template <>
InputParameters
validParams<Elastic>()
{
  InputParameters params = validParams<SolidModel>();
  return params;
}

Elastic::Elastic(const InputParameters & parameters) : SolidModel(parameters)
{

  createConstitutiveModel("ElasticModel");
}

////////////////////////////////////////////////////////////////////////

Elastic::~Elastic() {}

////////////////////////////////////////////////////////////////////////

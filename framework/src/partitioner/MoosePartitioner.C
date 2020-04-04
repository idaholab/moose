//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoosePartitioner.h"
#include "FEProblem.h"

InputParameters
MoosePartitioner::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addPrivateParam<MooseMesh *>("mesh");
  params.registerBase("MoosePartitioner");
  return params;
}

MoosePartitioner::MoosePartitioner(const InputParameters & params)
  : Partitioner(), MooseObject(params), Restartable(this, "Partitioners")
{
}

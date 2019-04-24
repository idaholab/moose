//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AuxKernelBasePD.h"
#include "PeridynamicsMesh.h"

template <>
InputParameters
validParams<AuxKernelBasePD>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Peridynamic AuxKernel base class");

  return params;
}

AuxKernelBasePD::AuxKernelBasePD(const InputParameters & parameters)
  : AuxKernel(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension())
{
}

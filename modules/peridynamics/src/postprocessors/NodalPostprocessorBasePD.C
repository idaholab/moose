//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalPostprocessorBasePD.h"

InputParameters
NodalPostprocessorBasePD::validParams()
{
  InputParameters params = NodalPostprocessor::validParams();
  params.addClassDescription("Base class for peridynamic nodal Postprocessors");

  return params;
}

NodalPostprocessorBasePD::NodalPostprocessorBasePD(const InputParameters & parameters)
  : NodalPostprocessor(parameters),
    _pdmesh(dynamic_cast<PeridynamicsMesh &>(_mesh)),
    _dim(_pdmesh.dimension())
{
}

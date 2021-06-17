//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLowerDVolumes.h"

#include "Assembly.h"
#include "MooseMesh.h"
#include "MooseUtils.h"

registerMooseObject("MooseTestApp", TestLowerDVolumes);

InputParameters
TestLowerDVolumes::validParams()
{
  InputParameters params = HFEMDiffusion::validParams();
  params.addRequiredParam<unsigned int>("n", "The number of elements in each direction");
  params.addRequiredParam<Real>("l", "The mesh side length in each direction");
  return params;
}

TestLowerDVolumes::TestLowerDVolumes(const InputParameters & parameters)
  : HFEMDiffusion(parameters),
    _lower_d_vol(_assembly.lowerDElemVolume()),
    _h(getParam<Real>("l") / getParam<unsigned int>("n")),
    _area(_mesh.dimension() == 3 ? (_h * _h) : (_mesh.dimension() == 2 ? _h : Real(1)))
{
}

void
TestLowerDVolumes::computeResidual()
{
  mooseAssert(MooseUtils::relativeFuzzyEqual(_area, _lower_d_vol),
              "These quantities must be equal");

  HFEMDiffusion::computeResidual();
}

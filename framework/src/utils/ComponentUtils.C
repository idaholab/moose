//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentUtils.h"
#include "InputParameters.h"
#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

#include "libmesh/vector_value.h"

InputParameters
ComponentUtils<Real>::validParams()
{
  return emptyInputParameters();
}

ComponentUtils<Real>::ComponentUtils(const InputParameters &) {}

Real
ComponentUtils<Real>::getComponent(const Real & v)
{
  return v;
}

InputParameters
ComponentUtils<RealVectorValue>::validParams()
{
  auto params = ComponentUtils<Real>::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>("i", "i<3", "The first component to extract");
  return params;
}

ComponentUtils<RealVectorValue>::ComponentUtils(const InputParameters & params)
  : ComponentUtils<Real>(params), _component_i(params.get<unsigned int>("i"))
{
}

Real
ComponentUtils<RealVectorValue>::getComponent(const RealVectorValue & v)
{
  return v(_component_i);
}

InputParameters
ComponentUtils<RankTwoTensor>::validParams()
{
  auto params = ComponentUtils<RealVectorValue>::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>("j", "j<3", "The second component to extract");
  return params;
}

ComponentUtils<RankTwoTensor>::ComponentUtils(const InputParameters & params)
  : ComponentUtils<RealVectorValue>(params), _component_j(params.get<unsigned int>("j"))
{
}

Real
ComponentUtils<RankTwoTensor>::getComponent(const RankTwoTensor & v)
{
  return v(_component_i, _component_j);
}

InputParameters
ComponentUtils<RankThreeTensor>::validParams()
{
  auto params = ComponentUtils<RankTwoTensor>::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>("k", "k<3", "The third component to extract");
  return params;
}

ComponentUtils<RankThreeTensor>::ComponentUtils(const InputParameters & params)
  : ComponentUtils<RankTwoTensor>(params), _component_k(params.get<unsigned int>("k"))
{
}

Real
ComponentUtils<RankThreeTensor>::getComponent(const RankThreeTensor & v)
{
  return v(_component_i, _component_j, _component_k);
}

InputParameters
ComponentUtils<RankFourTensor>::validParams()
{
  auto params = ComponentUtils<RankThreeTensor>::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>("l", "l<3", "The fourth component to extract");
  return params;
}

ComponentUtils<RankFourTensor>::ComponentUtils(const InputParameters & params)
  : ComponentUtils<RankThreeTensor>(params), _component_l(params.get<unsigned int>("l"))
{
}

Real
ComponentUtils<RankFourTensor>::getComponent(const RankFourTensor & v)
{
  return v(_component_i, _component_j, _component_k, _component_l);
}

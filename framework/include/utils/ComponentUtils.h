//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentUtils.h"

namespace ComponentUtils
{

ComponentReal::ComponentReal(const & InputParameters params) {}
Real
ComponentReal::getComponent(const Real & v)
{
  return v;
}

ComponentRealVectorValue::ComponentRealVectorValue(const & InputParameters params)
  : ComponentReal(params), _component_i(params.getParam<unsigned int>("i"))
{
}

Real
ComponentRealVectorValue::getComponent(const Real & v)
{
  return v(_component_i);
}

ComponentRankTwoTensor::ComponentRankTwoTensor(const & InputParameters params)
  : ComponentRealVectorValue(params), _component_j(params.getParam<unsigned int>("j"))
{
}

Real
ComponentRankTwoTensor::getComponent(const RankTwoTensor & v)
{
  return v(_component_i, _component_j);
}

ComponentRankThreeTensor::ComponentRankThreeTensor(const & InputParameters params)
  : ComponentRankTwoTensor(params), _component_k(params.getParam<unsigned int>("k"))
{
}

Real
ComponentRankThreeTensor::getComponent(const RankThreeTensor & v)
{
  return v(_component_i, _component_j, _component_k);
}

ComponentRankFourTensor::ComponentRankFourTensor(const & InputParameters params)
  : ComponentRankThreeTensor(params), _component_l(params.getParam<unsigned int>("l"))
{
}

Real
ComponentRankFourTensor::getComponent(const ComponentRankFourTensor & v)
{
  return v(_component_i, _component_j, _component_k, _component_l);
}

} // namespace ComponentUtils

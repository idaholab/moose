//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

template <typename T>
struct ComponentHelper;

template <>
struct ComponentHelper<Real>
{
  typedef ComponentUtils::ComponentReal type;
};

namespace ComponentUtils
{

class ComponentReal
{
protected:
  ComponentReal(const & InputParameters params);
  getComponent(const Real & v);
};

class ComponentRealVectorValue : public ComponentReal
{
protected:
  ComponentRealVectorValue(const & InputParameters params);
  Real getComponent(const Real & v);

  unsigned int _component_i;
};

class ComponentRankTwoTensor : public ComponentRealVectorValue
{
protected:
  ComponentRankTwoTensor(const & InputParameters params);
  Real getComponent(const RankTwoTensor & v);

  unsigned int _component_j;
};

class ComponentRankThreeTensor : public ComponentRankTwoTensor
{
protected:
  ComponentRankThreeTensor(const & InputParameters params);
  Real getComponent(const RankThreeTensor & v);

  unsigned int _component_k;
};

class ComponentRankFourTensor : public ComponentRankThreeTensor
{
protected:
  ComponentRankFourTensor(const & InputParameters params);
  Real getComponent(const ComponentRankFourTensor & v);

  unsigned int _component_l;
};

} // namespace ComponentUtils

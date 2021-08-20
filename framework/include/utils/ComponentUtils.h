//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

class InputParameters;

template <typename T>
class ComponentUtils;

template <>
class ComponentUtils<Real>
{
public:
  static InputParameters validParams();

protected:
  ComponentUtils(const InputParameters & params);
  Real getComponent(const Real & v);
};

template <>
class ComponentUtils<RealVectorValue> : public ComponentUtils<Real>
{
public:
  static InputParameters validParams();

protected:
  ComponentUtils(const InputParameters & params);
  Real getComponent(const RealVectorValue & v);

  unsigned int _component_i;
};

template <>
class ComponentUtils<RankTwoTensor> : public ComponentUtils<RealVectorValue>
{
public:
  static InputParameters validParams();

protected:
  ComponentUtils(const InputParameters & params);
  Real getComponent(const RankTwoTensor & v);

  unsigned int _component_j;
};

template <>
class ComponentUtils<RankThreeTensor> : public ComponentUtils<RankTwoTensor>
{
public:
  static InputParameters validParams();

protected:
  ComponentUtils(const InputParameters & params);
  Real getComponent(const RankThreeTensor & v);

  unsigned int _component_k;
};

template <>
class ComponentUtils<RankFourTensor> : public ComponentUtils<RankThreeTensor>
{
public:
  static InputParameters validParams();

protected:
  ComponentUtils(const InputParameters & params);
  Real getComponent(const RankFourTensor & v);

  unsigned int _component_l;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "DiracKernel.h"

class Function;

class XFEMPressure : public DiracKernel
{
public:
  static InputParameters validParams();

  XFEMPressure(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const int _component;
  const Real _factor;
  const Function * const _function;

  std::map<unsigned int, std::shared_ptr<ElementPairLocator>> * _element_pair_locators;
  std::map<const Elem *, std::map<unsigned int, Point>> _elem_qp_normal;
  std::map<const Elem *, std::map<unsigned int, Real>> _elem_qp_JxW;
};

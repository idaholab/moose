//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef XFEMPRESSURE_H
#define XFEMPRESSURE_H

// Moose Includes
#include "DiracKernel.h"

class Function;

class XFEMPressure : public DiracKernel
{
public:
  XFEMPressure(const InputParameters & parameters);

  virtual void addPoints();
  virtual Real computeQpResidual();

protected:
  const int _component;
  const Real _factor;
  Function * const _function;

  std::map<unsigned int, MooseSharedPointer<ElementPairLocator>> * _element_pair_locators;
  std::map<const Elem *, std::map<unsigned int, Point>> _elem_qp_normal;
  std::map<const Elem *, std::map<unsigned int, Real>> _elem_qp_JxW;
};

template <>
InputParameters validParams<XFEMPressure>();

#endif // XFEMPRESSURE_H

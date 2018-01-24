//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTINTEGRALMATERIALPROPERTY_H
#define ELEMENTINTEGRALMATERIALPROPERTY_H

#include "ElementIntegralPostprocessor.h"

class ElementIntegralMaterialProperty;

template <>
InputParameters validParams<ElementIntegralMaterialProperty>();

class ElementIntegralMaterialProperty : public ElementIntegralPostprocessor
{
public:
  ElementIntegralMaterialProperty(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  const MaterialProperty<Real> & _scalar;
};

#endif

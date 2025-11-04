//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"

class ElementSidePP : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  ElementSidePP(const InputParameters & parameters);

protected:
  virtual Real getValue() const override;

  virtual Real computeQpIntegral() override;

  const PostprocessorValue & _sidepp;
};

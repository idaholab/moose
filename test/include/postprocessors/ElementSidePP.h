//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ElementSidePP_H
#define ElementSidePP_H

#include "ElementIntegralPostprocessor.h"

// Forward Declarations
class ElementSidePP;

template <>
InputParameters validParams<ElementSidePP>();

class ElementSidePP : public ElementIntegralPostprocessor
{
public:
  ElementSidePP(const InputParameters & parameters);

protected:
  virtual Real getValue();

  virtual Real computeQpIntegral();

  const PostprocessorValue & _sidepp;
};

#endif // ElementSidePP_H

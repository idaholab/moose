//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef BREAKBOUNDARYONSUBDOMAIN_H
#define BREAKBOUNDARYONSUBDOMAIN_H

#include "MeshModifier.h"

class BreakBoundaryOnSubdomain;

template <>
InputParameters validParams<BreakBoundaryOnSubdomain>();

class BreakBoundaryOnSubdomain : public MeshModifier
{
public:
  BreakBoundaryOnSubdomain(const InputParameters & parameters);

  virtual void modify();
};

#endif /* BREAKBOUNDARYONSUBDOMAIN_H */

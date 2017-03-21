/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

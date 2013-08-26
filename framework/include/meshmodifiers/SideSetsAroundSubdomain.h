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

#ifndef SIDESETSAROUNDSUBDOMAIN_H
#define SIDESETSAROUNDSUBDOMAIN_H

#include "AddSideSetsBase.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictableRequired.h"

class SideSetsAroundSubdomain;

template<>
InputParameters validParams<SideSetsAroundSubdomain>();

class SideSetsAroundSubdomain :
  public MeshModifier,
  public BlockRestrictable,
  public BoundaryRestrictableRequired
{
public:
  SideSetsAroundSubdomain(const std::string & name, InputParameters parameters);

  virtual ~SideSetsAroundSubdomain();

  virtual void modify();

protected:

};

#endif /* SIDESETSAROUNDSUBDOMAIN_H */

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

#ifndef SIDESETSBETWEENSUBDOMAINS_H
#define SIDESETSBETWEENSUBDOMAINS_H

#include "AddSideSetsBase.h"
#include "BoundaryRestrictableRequired.h"

class SideSetsBetweenSubdomains;

template<>
InputParameters validParams<SideSetsBetweenSubdomains>();

class SideSetsBetweenSubdomains :
  public MeshModifier,
  public BoundaryRestrictableRequired
{
public:
  SideSetsBetweenSubdomains(const std::string & name, InputParameters parameters);

  virtual ~SideSetsBetweenSubdomains();

  virtual void modify();

protected:

};

#endif /* SIDESETSBETWEENSUBDOMAINS_H */

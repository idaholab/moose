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

#ifndef ONOFFDIRICHLETBC_H
#define ONOFFDIRICHLETBC_H

#include "DirichletBC.h"

class OnOffDirichletBC;

template<>
InputParameters validParams<OnOffDirichletBC>();

/**
 *
 */
class OnOffDirichletBC : public DirichletBC
{
public:
  OnOffDirichletBC(const std::string & name, InputParameters parameters);
  virtual ~OnOffDirichletBC();

  virtual bool shouldApply();

protected:
};

#endif /* ONOFFDIRICHLETBC_H */

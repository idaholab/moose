//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ONOFFDIRICHLETBC_H
#define ONOFFDIRICHLETBC_H

#include "DirichletBC.h"

class OnOffDirichletBC;

template <>
InputParameters validParams<OnOffDirichletBC>();

/**
 *
 */
class OnOffDirichletBC : public DirichletBC
{
public:
  OnOffDirichletBC(const InputParameters & parameters);
  virtual ~OnOffDirichletBC();

  virtual bool shouldApply();

protected:
};

#endif /* ONOFFDIRICHLETBC_H */

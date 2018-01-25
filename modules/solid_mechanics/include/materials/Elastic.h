//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELASTIC_H
#define ELASTIC_H

#include "SolidModel.h"

class Elastic;

template <>
InputParameters validParams<Elastic>();

class Elastic : public SolidModel
{
public:
  Elastic(const InputParameters & parameters);
  virtual ~Elastic();

protected:
};

#endif

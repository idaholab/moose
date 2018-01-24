//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSNOPENETRATIONBC_H
#define NSNOPENETRATIONBC_H

#include "MooseObject.h"

class NSNoPenetrationBC;

template <>
InputParameters validParams<NSNoPenetrationBC>();

/**
 * This class facilitates adding solid wall "no penetration" BCs for
 * the Euler equations.
 */
class NSNoPenetrationBC : public MooseObject
{
public:
  NSNoPenetrationBC(const InputParameters & parameters);
  virtual ~NSNoPenetrationBC();

protected:
};

#endif

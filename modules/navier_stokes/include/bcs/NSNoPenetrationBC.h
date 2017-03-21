/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

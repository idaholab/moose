//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSACTION_H
#define NSACTION_H

#include "Action.h"

class NSAction;

template <>
InputParameters validParams<NSAction>();

/**
 * This is a base Action class for the Navier-Stokes module which is
 * responsible for building lists of names that other Actions can
 * subsequently use.  Subclasses should call its act() function prior
 * to doing their own work.
 */
class NSAction : public Action
{
public:
  NSAction(InputParameters parameters);
  virtual ~NSAction();

  virtual void act();

protected:
  std::vector<std::string> _vars;
  std::vector<std::string> _auxs;

  // The Mesh dimension.  Derived classes may need to this when adding
  // variables and Kernels.
  unsigned int _dim;

  // Type that we use in Actions for declaring coupling
  typedef std::vector<VariableName> CoupledName;
};

#endif

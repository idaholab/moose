/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

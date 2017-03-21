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
#ifndef TEJUMPBC_H
#define TEJUMPBC_H

#include "NodalBC.h"

class TEJumpBC;
class Function;

template <>
InputParameters validParams<TEJumpBC>();

/**
 * Implements a BC for TimeError test case
 *
 */

class TEJumpBC : public NodalBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  TEJumpBC(const InputParameters & parameters);

  virtual ~TEJumpBC() {}

protected:
  virtual Real computeQpResidual();

  Real _t_jump;
  Real _slope;
};

#endif

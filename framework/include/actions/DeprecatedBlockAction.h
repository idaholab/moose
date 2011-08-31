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

#ifndef DEPRECATEDBLOCKACTION_H
#define DEPRECATEDBLOCKACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "Action.h"

class DeprecatedBlockAction;

template<>
InputParameters validParams<DeprecatedBlockAction>();

/**
 * Used for marking that some block are deprecated and not be used
 */
class DeprecatedBlockAction : public Action
{
public:
  DeprecatedBlockAction(const std::string & name, InputParameters parameters);
  virtual ~DeprecatedBlockAction();

  void act();

protected:

};

#endif // DEPRECATEDBLOCKACTION_H

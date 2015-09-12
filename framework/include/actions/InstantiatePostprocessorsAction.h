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

#ifndef INSTANTIATEPOSTPROCESSORSACTION_H
#define INSTANTIATEPOSTPROCESSORSACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class InstantiatePostprocessorsAction;

template<>
InputParameters validParams<InstantiatePostprocessorsAction>();

class InstantiatePostprocessorsAction : public Action
{
public:

  InstantiatePostprocessorsAction(InputParameters params);

  virtual ~InstantiatePostprocessorsAction();

  virtual void act();
};

#endif //INSTANTIATEPOSTPROCESSORSACTION_H

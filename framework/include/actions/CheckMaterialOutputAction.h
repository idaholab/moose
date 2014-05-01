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

#ifndef CHECKMATERIALOUTPUTACTION_H
#define CHECKMATERIALOUTPUTACTION_H

// MOOSE includes
#include "Action.h"

// Forward declerations
class CheckMaterialOutputAction;

template<>
InputParameters validParams<CheckMaterialOutputAction>();

/**
 * Action for checking that "outputs" is properly populated for Materials
 */
class CheckMaterialOutputAction : public Action
{
public:

  /**
   * Class constructor
   * @param name Name of this action
   * @param params Input parameters for this object
   */
  CheckMaterialOutputAction(const std::string & name, InputParameters params);

  /**
   * Class destructor
   */
  virtual ~CheckMaterialOutputAction();

  /**
   * Preforms a check on each of the Material objects that the "outputs" parameters has valid values
   */
  virtual void act();
};

#endif //CHECKMATERIALOUTPUTACTION_H

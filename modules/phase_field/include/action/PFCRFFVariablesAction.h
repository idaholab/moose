/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PFCRFFVARIABLESACTION_H
#define PFCRFFVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"

// Forward Declarations
class PFCRFFVariablesAction;

template <>
InputParameters validParams<PFCRFFVariablesAction>();

/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */
class PFCRFFVariablesAction : public Action
{
public:
  PFCRFFVariablesAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _num_L;
  const std::string _L_name_base;
};

#endif // PFCRFFVARIABLESACTION_H

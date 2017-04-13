/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHPFCRFFSPLITVARIABLESACTION_H
#define CHPFCRFFSPLITVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"

// Forward Declarations
class CHPFCRFFSplitVariablesAction;

template <>
InputParameters validParams<CHPFCRFFSplitVariablesAction>();

/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */
class CHPFCRFFSplitVariablesAction : public Action
{
public:
  CHPFCRFFSplitVariablesAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _num_L;
  const std::string _L_name_base;
  const std::vector<FileName> _sub_filenames;
  const AuxVariableName _n_name;
};

#endif // CHPFCRFFSPLITVARIABLESACTION_H

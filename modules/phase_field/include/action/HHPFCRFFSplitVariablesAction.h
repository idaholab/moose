#ifndef HHPFCRFFSPLITVARIABLESACTION_H
#define HHPFCRFFSPLITVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"

// Forward Declarations
class HHPFCRFFSplitVariablesAction;

template <>
InputParameters validParams<HHPFCRFFSplitVariablesAction>();

/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */
class HHPFCRFFSplitVariablesAction : public Action
{
public:
  HHPFCRFFSplitVariablesAction(const InputParameters & params);

  virtual void act();

private:
  const unsigned int _num_L;
  const std::string _L_name_base;
};

#endif // HHPFCRFFSPLITVARIABLESACTION_H

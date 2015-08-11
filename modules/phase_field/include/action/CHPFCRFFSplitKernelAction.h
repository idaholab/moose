#ifndef CHPFCRFFSPLITKERNELACTION_H
#define CHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

// Forward Declarations
class CHPFCRFFSplitKernelAction;

template<>
InputParameters validParams<CHPFCRFFSplitKernelAction>();

/**
 * \todo Needs documentation.
 */
class CHPFCRFFSplitKernelAction: public Action
{
public:
  CHPFCRFFSplitKernelAction(const InputParameters & params);

  virtual void act();

private:
  unsigned int _num_L;
  std::string _L_name_base;
  NonlinearVariableName _n_name;
};

#endif //CHPFCRFFSPLITKERNELACTION_H

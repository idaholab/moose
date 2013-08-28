#ifndef HHPFCRFFSPLITVARIABLESACTION_H
#define HHPFCRFFSPLITVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"
/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */

class HHPFCRFFSplitVariablesAction: public Action
{
public:
  HHPFCRFFSplitVariablesAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _num_L;
  std::string _L_name_base;
};

template<>
InputParameters validParams<HHPFCRFFSplitVariablesAction>();

#endif //HHPFCRFFSPLITVARIABLESACTION_H

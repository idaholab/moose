#ifndef PFCRFFVARIABLESACTION_H
#define PFCRFFVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"
/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */

class PFCRFFVariablesAction: public Action
{
public:
  PFCRFFVariablesAction(const std::string & name, InputParameters params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _num_L;
  std::string _L_name_base;
};

template<>
InputParameters validParams<PFCRFFVariablesAction>();

#endif //PFCRFFVARIABLESACTION_H

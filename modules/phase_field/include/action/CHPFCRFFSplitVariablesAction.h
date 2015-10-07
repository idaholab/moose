#ifndef CHPFCRFFSPLITVARIABLESACTION_H
#define CHPFCRFFSPLITVARIABLESACTION_H

#include "InputParameters.h"
#include "Action.h"

//Forward Declarations
class CHPFCRFFSplitVariablesAction;

template<>
InputParameters validParams<CHPFCRFFSplitVariablesAction>();

/**
 * Automatically generates all the L variables for the RFF phase field crystal model.
 */
class CHPFCRFFSplitVariablesAction: public Action
{
public:
  CHPFCRFFSplitVariablesAction(const InputParameters & params);

  virtual void act();

private:
  static const Real _abs_zero_tol;

  unsigned int _num_L;
  std::string _L_name_base;
  std::vector<FileName> _sub_filenames;
  AuxVariableName _n_name;
};

#endif //CHPFCRFFSPLITVARIABLESACTION_H

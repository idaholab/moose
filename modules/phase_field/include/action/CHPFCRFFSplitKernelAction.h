#ifndef CHPFCRFFSPLITKERNELACTION_H
#define CHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

class CHPFCRFFSplitKernelAction: public Action
{
public:
  CHPFCRFFSplitKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:

  unsigned int _num_L;
  std::string _L_name_base;
  std::string _n_name;
  

};

template<>
InputParameters validParams<CHPFCRFFSplitKernelAction>();
  
#endif //CHPFCRFFSPLITKERNELACTION_H

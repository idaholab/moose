#ifndef HHPFCRFFSPLITKERNELACTION_H
#define HHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

class HHPFCRFFSplitKernelAction: public Action
{
public:
  HHPFCRFFSplitKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:

  unsigned int _num_L;
  std::string _L_name_base;
  std::string _n_name;
  

};

template<>
InputParameters validParams<HHPFCRFFSplitKernelAction>();
  
#endif //HHPFCRFFSPLITKERNELACTION_H

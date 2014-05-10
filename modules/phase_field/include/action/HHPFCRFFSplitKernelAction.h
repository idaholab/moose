#ifndef HHPFCRFFSPLITKERNELACTION_H
#define HHPFCRFFSPLITKERNELACTION_H

#include "Action.h"

//Forward Declarations
class HHPFCRFFSplitKernelAction;

template<>
InputParameters validParams<HHPFCRFFSplitKernelAction>();

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

#endif //HHPFCRFFSPLITKERNELACTION_H

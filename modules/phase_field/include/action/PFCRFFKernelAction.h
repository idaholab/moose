#ifndef PFCRFFKERNELACTION_H
#define PFCRFFKERNELACTION_H

#include "Action.h"

class PFCRFFKernelAction: public Action
{
public:
  PFCRFFKernelAction(const std::string & name, InputParameters params);

  virtual void act();

private:

  unsigned int _num_L;
  std::string _L_name_base;
  std::string _n_name;
  

};

template<>
InputParameters validParams<PFCRFFKernelAction>();
  
#endif //PFCRFFKERNELACTION_H

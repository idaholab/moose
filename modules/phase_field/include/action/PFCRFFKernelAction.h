/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef PFCRFFKERNELACTION_H
#define PFCRFFKERNELACTION_H

#include "Action.h"

//Forward Declarations
class PFCRFFKernelAction;

template<>
InputParameters validParams<PFCRFFKernelAction>();

class PFCRFFKernelAction: public Action
{
public:
  PFCRFFKernelAction(const InputParameters & params);

  virtual void act();

protected:
  const unsigned int _num_L;
  const std::string _L_name_base;
  const std::string _n_name;
};

#endif //PFCRFFKERNELACTION_H

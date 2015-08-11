/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RECONVARICACTION_H
#define RECONVARICACTION_H

#include "InputParameters.h"
#include "Action.h"
//#include "EBSDReader.h"

// Forward Declarations
class ReconVarICAction;

template<>
InputParameters validParams<ReconVarICAction>();

class ReconVarICAction: public Action
{
public:
  ReconVarICAction(const InputParameters & params);

  virtual void act();

private:
  unsigned int _op_num;
  std::string _var_name_base;
  //std::vector<VariableName> _eta;
};

#endif //RECONVARICACTION_H

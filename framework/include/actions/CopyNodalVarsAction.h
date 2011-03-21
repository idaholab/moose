#ifndef COPYNODALVARSACTION_H
#define COPYNODALVARSACTION_H

#include "AddVariableAction.h"

class CopyNodalVarsAction: public Action
{
public:
  CopyNodalVarsAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<CopyNodalVarsAction>();

#endif //COPYNODALVARSACTION_H

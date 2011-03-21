#ifndef ADDMESHMODIFIERACTION_H
#define ADDMESHMODIFIERACTION_H

#include "MooseObjectAction.h"

class AddMeshModifierAction: public MooseObjectAction
{
public:
  AddMeshModifierAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddMeshModifierAction>();

#endif //ADDMESHMODIFIERACTION_H

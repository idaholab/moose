#ifndef ADDMATERIALACTION_H
#define ADDMATERIALACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddMaterialAction : public MooseObjectAction
{
public:
  AddMaterialAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddMaterialAction>();

#endif // ADDMATERIALACTION_H

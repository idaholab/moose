#ifndef ADDBOUNDSVECTORSACTION_H
#define ADDBOUNDSVECTORSACTION_H

#include "InputParameters.h"
#include "Moose.h"
#include "MooseObjectAction.h"

#include <string>

class AddBoundsVectorsAction : public Action
{
public:
  AddBoundsVectorsAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddBoundsVectorsAction>();

#endif // ADDBOUNDSVECTORSACTION_H

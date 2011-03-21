#ifndef ADDFUNCTIONACTION_H_
#define ADDFUNCTIONACTION_H_

#include "MooseObjectAction.h"

/**
 * This class parses functions in the [Functions] block and creates them.
 */
class AddFunctionAction : public MooseObjectAction
{
public:
  AddFunctionAction(const std::string & name, InputParameters params);

  virtual void act();
};

template<>
InputParameters validParams<AddFunctionAction>();

#endif //ADDFUNCTIONACTION_H_

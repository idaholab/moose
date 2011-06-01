#ifndef PRESSURERZACTION_H
#define PRESSURERZACTION_H

#include "PressureAction.h"

class PressureRZAction;

template<>
InputParameters validParams<PressureRZAction>();

class PressureRZAction: public PressureAction
{
public:
  PressureRZAction(const std::string & name, InputParameters params);
};


#endif // PRESSURERZACTION_H

#ifndef PLENUMPRESSURERZACTION_H
#define PLENUMPRESSURERZACTION_H

#include "PlenumPressureAction.h"

class PlenumPressureRZAction;

template<>
InputParameters validParams<PlenumPressureRZAction>();

class PlenumPressureRZAction: public PlenumPressureAction
{
public:
  PlenumPressureRZAction(const std::string & name, InputParameters params);
};


#endif // PLENUMPRESSURERZACTION_H

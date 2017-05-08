#ifndef RELAP7CONTROL_H
#define RELAP7CONTROL_H

#include "Control.h"

class RELAP7Control;

template <>
InputParameters validParams<RELAP7Control>();

class RELAP7Control : public Control
{
public:
  RELAP7Control(const InputParameters & parameters);

protected:
};

#endif // RELAP7CONTROL_H

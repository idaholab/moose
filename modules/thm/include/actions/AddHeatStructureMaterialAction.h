#ifndef ADDHEATSTRUCTUREMATERIALACTION_H
#define ADDHEATSTRUCTUREMATERIALACTION_H

#include "AddUserObjectAction.h"

class AddHeatStructureMaterialAction;

template <>
InputParameters validParams<AddHeatStructureMaterialAction>();

class AddHeatStructureMaterialAction : public AddUserObjectAction
{
public:
  AddHeatStructureMaterialAction(InputParameters params);
};

#endif /* ADDHEATSTRUCTUREMATERIALACTION_H */

#include "AddFluidPropertiesAction.h"

template<>
InputParameters validParams<AddFluidPropertiesAction>()
{
  return validParams<AddUserObjectAction>();
}

AddFluidPropertiesAction::AddFluidPropertiesAction(InputParameters params) :
    AddUserObjectAction(params)
{
}

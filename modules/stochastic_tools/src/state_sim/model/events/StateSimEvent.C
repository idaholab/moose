/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "StateSimEvent.h"
#include "StateSimModel.h"
#include <string>
#include <typeinfo>
#include <unordered_map>

StateSimEvent::StateSimEvent(StateSimModel & main_model, const std::string & name, EVENT_TYPE_ENUM ev_type)
  : StateSimBase(main_model.nextID(STATESIM_TYPE::EVENT), name),
    _ev_type(ev_type)
{
}

StateSimEvent::~StateSimEvent()
{
} //still required for virtual destructor

void
StateSimEvent::addRelatedItem(StateSimBase & item)
{
  //Child classes need to make sure the correct type of items are being added
  //mooseAssert(_related_items_type == typeid(item), "StateSimEvent addRelatedItem() - A realted item with the same ID but differnt type already exists.");

  _related_items[item.id()] = &item;
}

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMALLEVENTS_H
#define STATESIMALLEVENTS_H

#include <string>
#include <unordered_map>
#include <map>
#include "StateSimEvent.h"
#include "StateSimObjList.h"


/**
 * Storage and Lookup list class for all model Events.
 */
class StateSimAllEvents : public StateSimObjList<StateSimEvent>//std::shared_ptr<StateSimEvent>>
{
  //todo
  //static StateSimEvent createSimEvent(EVENT_TYPE_ENUM act_type)
  //virtual std::string getJSON(bool incBrackets, LookupLists lists);
  //virtual bool deserializeJSON(? json_obj, LookupLists lists, bool use_given_ids);
  //virtual bool loadObjLinks(? json_obj, LookupLists lists); //load any links to other objects after initial lists are loaded.

};

#endif

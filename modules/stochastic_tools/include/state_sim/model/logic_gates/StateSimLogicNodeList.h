/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef STATESIMLOGICNODELIST_H
#define STATESIMLOGICNODELIST_H

#include <string>
#include <unordered_map>
#include <map>
#include "StateSimLogicNode.h"
#include "StateSimObjList.h"

/**
 * Logic Node Lists to store all the logic nodes of a model.
 */
class StateSimLogicNodeList : public StateSimObjList<StateSimLogicNode>
{
  //todo
  //static StateSimEvent createSimEvent(EVENT_TYPE_ENUM act_type)
  //virtual std::string getJSON(bool incBrackets, LookupLists lists);
  //virtual bool deserializeJSON(? json_obj, LookupLists lists, bool use_given_ids);
  //virtual bool loadObjLinks(? json_obj, LookupLists lists); //load any links to other objects after initial lists are loaded.
};

#endif

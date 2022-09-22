//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRayTracingObjectAction.h"

// Local Includes
#include "RayTracingStudy.h"

// MOOSE includes
#include "TheWarehouse.h"

InputParameters
AddRayTracingObjectAction::validParams()
{
  return MooseObjectAction::validParams();
}

AddRayTracingObjectAction::AddRayTracingObjectAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddRayTracingObjectAction::act()
{
  RayTracingStudy * rts = nullptr;

  // Query into UserObjects
  std::vector<UserObject *> uos;
  auto query = _problem->theWarehouse().query().condition<AttribSystem>("UserObject");

  // Object has a study: see if it exists
  if (_moose_object_pars.isParamSetByUser("study"))
  {
    const auto study_name = _moose_object_pars.get<UserObjectName>("study");
    query.condition<AttribName>(study_name);
    query.queryInto(uos);

    if (uos.empty())
      mooseError(_type, " '", _name, "': Could not find the requested study '", study_name, "'.");

    rts = dynamic_cast<RayTracingStudy *>(uos[0]);
    if (!rts)
      mooseError(_type,
                 " '",
                 _name,
                 "' requested the study ",
                 study_name,
                 " but the provided study is not a RayTracingStudy-derived object.");
  }
  // Object doesn't have a study: find one and only one study to associate with it
  else
  {
    query.queryInto(uos);

    for (auto & uo : uos)
    {
      RayTracingStudy * rts_temp = dynamic_cast<RayTracingStudy *>(uo);
      if (rts_temp)
      {
        if (rts)
          mooseError("While constructing the ",
                     _type,
                     " '",
                     _name,
                     "', multiple RayTracingStudy objects were found.\n\nYou must associate one "
                     "of the RayTracingStudy objects by setting the 'study' parameter in ",
                     _type,
                     " '",
                     _name,
                     "'");

        rts = rts_temp;
      }
    }

    if (!rts)
      mooseError(
          _type,
          " '",
          _name,
          "' did not provide a RayTracingStudy to associate with via the 'study' parameter "
          "and a study was not found.\n\nYou must add a RayTracingStudy to use said object.");
  }

  _moose_object_pars.set<RayTracingStudy *>("_ray_tracing_study") = rts;

  addRayTracingObject();
}

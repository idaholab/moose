//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FEProblemBase.h"
#include "Function.h"
#include "MultiApp.h"
#include "UserObject.h"

#include "MultiAppFXTransfer.h"

registerMooseObject("FunctionalExpansionToolsApp", MultiAppFXTransfer);

InputParameters
MultiAppFXTransfer::validParams()
{
  InputParameters params = MultiAppTransfer::validParams();

  params.addClassDescription("Transfers coefficient arrays between objects that are derived from "
                             "MutableCoefficientsInterface; currently includes the following "
                             "types: FunctionSeries, FXBoundaryUserObject, and FXVolumeUserObject");

  params.addRequiredParam<std::string>(
      "this_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in this app (LocalApp).");

  params.addRequiredParam<std::string>(
      "multi_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in the MultiApp.");

  return params;
}

MultiAppFXTransfer::MultiAppFXTransfer(const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _this_app_object_name(getParam<std::string>("this_app_object_name")),
    _multi_app_object_name(getParam<std::string>("multi_app_object_name")),
    getMultiAppObject(NULL),
    getSubAppObject(NULL)
{
  if (_directions.size() != 1)
    paramError("direction", "This transfer is only unidirectional");
  if (hasToMultiApp() && hasFromMultiApp())
    mooseError("This transfer does not currently support between_multiapp transfer");
}

void
MultiAppFXTransfer::initialSetup()
{
  const auto multi_app = hasFromMultiApp() ? getFromMultiApp() : getToMultiApp();

  // Search for the _this_app_object_name in the LocalApp
  getMultiAppObject =
      scanProblemBaseForObject(multi_app->problemBase(), _this_app_object_name, "MultiApp");
  if (getMultiAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _this_app_object_name, "' in MultiApp");

  // Search for the _multi_app_object_name in each of the MultiApps
  for (std::size_t i = 0; i < multi_app->numGlobalApps(); ++i)
    if (multi_app->hasLocalApp(i))
    {
      if (i == 0) // First time through, assign without checking against previous values
        getSubAppObject = scanProblemBaseForObject(
            multi_app->appProblemBase(i), _multi_app_object_name, multi_app->name());
      else if (getSubAppObject != scanProblemBaseForObject(multi_app->appProblemBase(i),
                                                           _multi_app_object_name,
                                                           multi_app->name()))
        mooseError("The name '",
                   _multi_app_object_name,
                   "' is assigned to two different object types. Please modify your input file and "
                   "try again.");
    }
  if (getSubAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in SubApp");
}

MultiAppFXTransfer::GetProblemObject
MultiAppFXTransfer::scanProblemBaseForObject(FEProblemBase & base,
                                             const std::string & object_name,
                                             const std::string & app_name)
{
  /*
   * For now we are only considering Functions and UserObjects, as they are the only types currently
   * implemented with MutableCoefficientsInterface. Others may be added later.
   *
   * Functions:
   *   FunctionSeries
   *
   * UserObjects:
   *   FXBoundaryUserObject (via FXBaseUserObject)
   *   FXVolumeUserObject (via FXBaseUserObject)
   */
  MutableCoefficientsInterface * interface;

  // Check to see if the object with object_name is a Function
  if (base.hasFunction(object_name))
  {
    Function & function = base.getFunction(object_name);
    interface = dynamic_cast<MutableCoefficientsInterface *>(&function);

    // Check to see if the function is a subclass of MutableCoefficientsInterface
    if (interface)
      return &MultiAppFXTransfer::getMutableCoefficientsFunction;
    else
      mooseError("Function '",
                 object_name,
                 "' in '",
                 app_name,
                 "' does not inherit from MutableCoefficientsInterface.",
                 " Please change the function type and try again.");
  }
  // Check to see if the object with object_name is a UserObject
  else if (base.hasUserObject(object_name))
  {
    // Get the non-const qualified UserObject, otherwise we would use getUserObject()
    auto & user_object = base.getUserObject<UserObject>(object_name);
    interface = dynamic_cast<MutableCoefficientsInterface *>(&user_object);

    // Check to see if the userObject is a subclass of MutableCoefficientsInterface
    if (interface)
      return &MultiAppFXTransfer::getMutableCoefficientsUserOject;
    else
      mooseError("UserObject '",
                 object_name,
                 "' in '",
                 app_name,
                 "' does not inherit from MutableCoefficientsInterface.",
                 " Please change the function type and try again.");
  }

  return NULL;
}

MutableCoefficientsInterface &
MultiAppFXTransfer::getMutableCoefficientsFunction(FEProblemBase & base,
                                                   const std::string & object_name,
                                                   THREAD_ID thread)
{
  return dynamic_cast<MutableCoefficientsInterface &>(base.getFunction(object_name, thread));
}

MutableCoefficientsInterface &
MultiAppFXTransfer::getMutableCoefficientsUserOject(FEProblemBase & base,
                                                    const std::string & object_name,
                                                    THREAD_ID thread)
{
  // Get the non-const qualified UserObject, otherwise we would use getUserObject()
  auto & user_object = base.getUserObject<UserObject>(object_name, thread);
  return dynamic_cast<MutableCoefficientsInterface &>(user_object);
}

void
MultiAppFXTransfer::execute()
{
  _console << "Beginning MultiAppFXTransfer: " << name() << std::endl;

  switch (_current_direction)
  {
    // LocalApp -> MultiApp
    case TO_MULTIAPP:
    {
      // Get a reference to the object in the LocalApp
      const MutableCoefficientsInterface & from_object =
          (this->*getMultiAppObject)(getToMultiApp()->problemBase(), _this_app_object_name, 0);

      for (unsigned int i = 0; i < getToMultiApp()->numGlobalApps(); ++i)
      {
        if (getToMultiApp()->hasLocalApp(i))
          for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
          {
            // Get a reference to the object in each MultiApp
            MutableCoefficientsInterface & to_object = (this->*getSubAppObject)(
                getToMultiApp()->appProblemBase(i), _multi_app_object_name, t);

            if (to_object.isCompatibleWith(from_object))
              to_object.importCoefficients(from_object);
            else
              mooseError("'",
                         _multi_app_object_name,
                         "' is not compatible with '",
                         _this_app_object_name,
                         "'");
          }
      }
      break;
    }

    // MultiApp -> LocalApp
    case FROM_MULTIAPP:
    {
      /*
       * For now we will assume that the transfers are 1:1 and the coefficients are synchronized
       * among all instances, thus we only need to grab the set of coefficients from the first
       * SubApp.
       */
      if (getFromMultiApp()->hasLocalApp(0))
      {
        // Get a reference to the first thread object in the first MultiApp
        const MutableCoefficientsInterface & from_object = (this->*getSubAppObject)(
            getFromMultiApp()->appProblemBase(0), _multi_app_object_name, 0);

        for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
        {
          // Get a reference to the object in each LocalApp instance
          MutableCoefficientsInterface & to_object = (this->*getMultiAppObject)(
              getFromMultiApp()->problemBase(), _this_app_object_name, t);

          if (to_object.isCompatibleWith(from_object))
            to_object.importCoefficients(from_object);
          else
            mooseError("'",
                       _multi_app_object_name,
                       "' is not compatible with '",
                       _this_app_object_name,
                       "'");
        }
      }
      break;
    }
  }

  _console << "Finished MultiAppFXTransfer: " << name() << std::endl;
}

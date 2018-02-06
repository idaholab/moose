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

#include "MultiAppMutableCoefficientsTransfer.h"

template <>
InputParameters
validParams<MultiAppMutableCoefficientsTransfer>()
{
  InputParameters params = validParams<MultiAppTransfer>();

  params.addClassDescription("Transfers coefficient arrays between objects that are derived from "
                             "MutableCoefficientsInterface; currently includes the following "
                             "types: FunctionSeries, FXBoundaryUserObject, and FXVolumeUserObject");

  params.addRequiredParam<std::string>(
      "this_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in this app (LocalApp).");

  params.addRequiredParam<std::string>(
      "multi_app_object_name",
      "Name of the MutableCoefficientsInterface-derived object in the MultiApp.");

  params.addParam<std::string>(
      "pretty_name", "MultiAppFXTransfer", "Typename by which this class should be identified.");

  return params;
}

MultiAppMutableCoefficientsTransfer::MultiAppMutableCoefficientsTransfer(
    const InputParameters & parameters)
  : MultiAppTransfer(parameters),
    _pretty_name(getParam<std::string>("pretty_name")),
    _this_app_object_name(getParam<std::string>("this_app_object_name")),
    _multi_app_object_name(getParam<std::string>("multi_app_object_name")),
    getMultiAppObject(NULL),
    getSubAppObject(NULL)
{
  // Nothing here
}

void
MultiAppMutableCoefficientsTransfer::initialSetup()
{
  // Search for the _this_app_object_name in the LocalApp
  getMultiAppObject =
      scanProblemBaseForObject(_multi_app->problemBase(), _this_app_object_name, "MultiApp");
  if (getMultiAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in MultiApp");

  // Search for the _multi_app_object_name in each of the MultiApps
  for (std::size_t i = 0; i < _multi_app->numGlobalApps(); ++i)
    if (_multi_app->hasLocalApp(i))
    {
      if (i == 0) // First time through, assign without checking against previous values
        getSubAppObject = scanProblemBaseForObject(
            _multi_app->appProblemBase(i), _multi_app_object_name, _multi_app->name());
      else if (getSubAppObject != scanProblemBaseForObject(_multi_app->appProblemBase(i),
                                                           _multi_app_object_name,
                                                           _multi_app->name()))
        mooseError("The name '",
                   _multi_app_object_name,
                   "' is assigned to two different object types. Please modify your input file and "
                   "try again.");
    }
  if (getSubAppObject == NULL)
    mooseError(
        "Transfer '", name(), "': Cannot find object '", _multi_app_object_name, "' in SubApp");
}

MultiAppMutableCoefficientsTransfer::GetProblemObject
MultiAppMutableCoefficientsTransfer::scanProblemBaseForObject(FEProblemBase & base,
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
      return &MultiAppMutableCoefficientsTransfer::getMutableCoefficientsFunction;
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
    UserObject * user_object = base.getUserObjects().getActiveObject(object_name).get();
    interface = dynamic_cast<MutableCoefficientsInterface *>(user_object);

    // Check to see if the userObject is a subclass of MutableCoefficientsInterface
    if (interface)
      return &MultiAppMutableCoefficientsTransfer::getMutableCoefficientsUserOject;
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
MultiAppMutableCoefficientsTransfer::getMutableCoefficientsFunction(FEProblemBase & base,
                                                                    const std::string & object_name,
                                                                    THREAD_ID thread)
{
  return dynamic_cast<MutableCoefficientsInterface &>(base.getFunction(object_name, thread));
}

MutableCoefficientsInterface &
MultiAppMutableCoefficientsTransfer::getMutableCoefficientsUserOject(
    FEProblemBase & base, const std::string & object_name, THREAD_ID thread)
{
  // Get the non-const qualified UserObject, otherwise we would use getUserObject()
  UserObject * user_object = base.getUserObjects().getActiveObject(object_name, thread).get();

  return dynamic_cast<MutableCoefficientsInterface &>(*user_object);
}

void
MultiAppMutableCoefficientsTransfer::execute()
{
  _console << "Beginning " << _pretty_name << ": " << name() << std::endl;

  switch (_direction)
  {
    // LocalApp -> MultiApp
    case TO_MULTIAPP:
    {
      // Get a reference to the object in the LocalApp
      const MutableCoefficientsInterface & from_object =
          (this->*getMultiAppObject)(_multi_app->problemBase(), _this_app_object_name, 0);

      for (unsigned int i = 0; i < _multi_app->numGlobalApps(); ++i)
      {
        if (_multi_app->hasLocalApp(i))
          for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
          {
            // Get a reference to the object in each MultiApp
            MutableCoefficientsInterface & to_object =
                (this->*getSubAppObject)(_multi_app->appProblemBase(i), _multi_app_object_name, t);

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
      if (_multi_app->hasLocalApp(0))
      {
        // Get a reference to the first thread object in the first MultiApp
        const MutableCoefficientsInterface & from_object =
            (this->*getSubAppObject)(_multi_app->appProblemBase(0), _multi_app_object_name, 0);

        for (THREAD_ID t = 0; t < libMesh::n_threads(); ++t)
        {
          // Get a reference to the object in each LocalApp instance
          MutableCoefficientsInterface & to_object =
              (this->*getMultiAppObject)(_multi_app->problemBase(), _this_app_object_name, t);

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

  _console << "Finished " << _pretty_name << ": " << name() << std::endl;
}

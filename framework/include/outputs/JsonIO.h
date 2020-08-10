//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// The output to JSON is currently limited to Reporter objects via the ReporterContext object and
// the JSONOutput object.
//
// It is possible to create a general JSON based output of all RestartableData objects, which
// already includes the "store/load" method overloadeds that use a nlohmann::json objects. However,
// these function do nothing at the moment (see RestartableData<T>::store/load).
//
// The JSON library being used includes a "to_json" function that operates in the same fashion as
// our dataLoad/Store functions, including the automatic container handling.
//
// To enable output of RestartableData general the following needs to be done.
//
//     1. RestartableData<T>::store(nlohmann::json &) should call the storeHelper method loadted
//        in this file
//     2. RestartableData<T>::load(const nlohmann::json &) should call the loadHelper method
//        located in this file
//     3. Create the necessary load/store overloads for the various types must be created using the
//        "to_json" function
//     4. Implement a means via Checkpoint to use the JSON format

#pragma once

#include "nlohmann/json.h"

void to_json(nlohmann::json & json, const MooseApp & app);

template <typename T>
void storeHelper(nlohmann::json & json, const T & value, void * context = nullptr);

template <typename T>
void loadHelper(const nlohmann::json & json, T & value, void * context = nullptr);

template <typename T>
void
storeHelper(nlohmann::json & json, const T & value, void * /*context*/)
{
  nlohmann::to_json(json, value);
}

template <typename T>
void
loadHelper(const nlohmann::json & json, T & value, void * /*context*/)
{
  nlohmann::from_json(json, value);
}

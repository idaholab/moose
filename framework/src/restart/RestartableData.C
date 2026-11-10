//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableData.h"

RestartableDataValue::RestartableDataValue(const std::string & name, void * const context)
  : _name(name), _context(context), _declared(false), _loaded(false), _stored(false)
{
}

void
RestartableDataValue::setDeclared(const SetDeclaredKey)
{
  mooseAssert(!_declared, "Already declared");
  _declared = true;
}

void
RestartableDataValue::store(std::ostream & stream)
{
  storeInternal(stream);
  _stored = true;
}

void
RestartableDataValue::load(std::istream & stream)
{
  loadInternal(stream);
  _loaded = true;
}

void
RestartableDataValue::store(nlohmann::json & json, const StoreJSONParams & params) const
{
  if (params.value)
  {
    if (hasStoreJSON())
      storeJSONValue(json["value"]);
    else
      mooseError("Failed to output restartable data '",
                 name(),
                 "' as JSON because a to_json method is not implemented for the type '",
                 type(),
                 "'");
  }
  if (params.type)
    json["type"] = type();
  if (params.name)
    json["name"] = name();
  if (params.declared)
    json["declared"] = declared();
  if (params.loaded)
    json["loaded"] = loaded();
  if (params.stored)
    json["stored"] = stored();
  if (params.has_context)
    json["has_context"] = hasContext();
}

template <>
void
dataStore(std::ostream & stream, RestartableDataValue::StoreJSONParams & v, void * ctx)
{
  dataStore(stream, v.value, ctx);
  dataStore(stream, v.type, ctx);
  dataStore(stream, v.name, ctx);
  dataStore(stream, v.declared, ctx);
  dataStore(stream, v.loaded, ctx);
  dataStore(stream, v.stored, ctx);
  dataStore(stream, v.has_context, ctx);
}

template <>
void
dataLoad(std::istream & stream, RestartableDataValue::StoreJSONParams & v, void * ctx)
{
  dataLoad(stream, v.value, ctx);
  dataLoad(stream, v.type, ctx);
  dataLoad(stream, v.name, ctx);
  dataLoad(stream, v.declared, ctx);
  dataLoad(stream, v.loaded, ctx);
  dataLoad(stream, v.stored, ctx);
  dataLoad(stream, v.has_context, ctx);
}

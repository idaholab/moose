//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBase.h"

#include "MooseError.h"
#include "InputParameters.h"
#include "MooseApp.h"

#include "hit/parse.h"

MooseBase::MooseBase(const std::string & type,
                     const std::string & name,
                     MooseApp & app,
                     const InputParameters & params)
  : _app(app), _type(type), _name(name), _params(params)
{
}

std::string
MooseBase::typeAndName() const
{
  return type() + std::string(" \"") + name() + std::string("\"");
}

[[noreturn]] void
MooseBase::callMooseError(std::string msg, const bool with_prefix) const
{
  _app.getOutputWarehouse().mooseConsole();
  const std::string prefix = _app.isUltimateMaster() ? "" : _app.name();
  if (with_prefix)
    msg = errorPrefix("error") + msg;
  moose::internal::mooseErrorRaw(msg, prefix);
}

std::string
MooseBase::errorPrefix(const std::string & error_type) const
{
  std::stringstream oss;
  if (const auto node = _params.getHitNode())
    if (!node->isRoot())
      oss << node->fileLocation() << ":\n";
  oss << "The following " << error_type << " occurred in the ";
  if (const auto base_ptr = _params.getBase())
    oss << *base_ptr;
  else
    oss << "object";
  oss << " '" << name() << "' of type " << type() << ".\n\n";
  return oss.str();
}

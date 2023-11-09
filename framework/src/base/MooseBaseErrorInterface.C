//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseBaseErrorInterface.h"
#include "MooseBase.h"
#include "MooseApp.h"

std::string
MooseBaseErrorInterface::errorPrefix(const std::string & error_type) const
{
  std::stringstream oss;
  oss << "The following " << error_type << " occurred in the object \"" << _moose_base->name()
      << "\", of type \"" << _moose_base->type() << "\".\n\n";
  return oss.str();
}

[[noreturn]] void
callMooseErrorRaw(std::string & msg, MooseApp * app)
{
  app->getOutputWarehouse().mooseConsole();
  std::string prefix;
  if (!app->isUltimateMaster())
    prefix = app->name();
  moose::internal::mooseErrorRaw(msg, prefix);
}

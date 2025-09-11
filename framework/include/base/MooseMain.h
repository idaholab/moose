//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseInit.h"
#include "MooseApp.h"

class InputParameters;

namespace Moose
{

/**
 * Create a MooseApp from command-line arguments.
 */
std::unique_ptr<MooseApp>
createMooseApp(const std::string & default_app_type, int argc, char * argv[]);

/**
 * Initialize, create and run a MooseApp
 */
template <typename DefaultAppType>
int
main(int argc, char * argv[])
{
  MooseInit init(argc, argv);
  std::cout << "just inited from main " << libMesh::command_line_value("--n-threads", 1) << std::endl;

  DefaultAppType::registerApps();

  const auto default_app_type = MooseUtils::prettyCppType<DefaultAppType>();

  auto app = createMooseApp(default_app_type, argc, argv);

  app->run();

  return app->exitCode();
}
}

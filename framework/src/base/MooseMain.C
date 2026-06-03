//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseMain.h"
#include "AppFactory.h"

#ifdef LIBMESH_HAVE_OPENMP
#include <omp.h>
#endif
#include <regex>

namespace Moose
{

std::unique_ptr<MooseApp>
createMooseApp(const std::string & default_app_type, int argc, char * argv[])
{
  // Do not allow overriding Application/type= for subapps
  for (int i = 1; i < argc; ++i)
    if (std::regex_match(argv[i], std::regex("[A-Za-z0-9]*:Application/.*")))
      mooseError(
          "For command line argument '",
          argv[i],
          "': overriding the application type for MultiApps via command line is not allowed.");

  return AppFactory::create(default_app_type, std::vector<std::string>(argv + 1, argv + argc));
}
}

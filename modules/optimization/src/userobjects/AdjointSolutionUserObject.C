//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointSolutionUserObject.h"

#include <sys/stat.h>

registerMooseObject("OptimizationApp", AdjointSolutionUserObject);

InputParameters
AdjointSolutionUserObject::validParams()
{
  InputParameters params = SolutionUserObject::validParams();
  params.addClassDescription(
      "Reads a variable from a mesh in one simulation to another specifically for loading forward "
      "solution in adjoint simulation during inverse optimization.");

  params.addParam<Real>(
      "reverse_time_end",
      "End time used for reversing the time integration when evaluating function derivative.");

  return params;
}

AdjointSolutionUserObject::AdjointSolutionUserObject(const InputParameters & parameters)
  : SolutionUserObject(parameters),
    _reverse_time_end(isParamValid("reverse_time_end") ? &getParam<Real>("reverse_time_end")
                                                       : nullptr),
    _file_mod_time(std::numeric_limits<std::time_t>::min())
{
  if (isParamValid("reverse_time_end") &&
      !MooseUtils::hasExtension(_mesh_file, "e", /*strip_exodus_ext =*/true))
    paramError("reverse_time_end",
               "Performing transient adjoint simulation currently only works if the forward "
               "solution is written and read from exodus format.");
}

void
AdjointSolutionUserObject::timestepSetup()
{
  // Re-read the mesh file if it has been modified
  struct stat stats;
  stat(_mesh_file.c_str(), &stats);
  if (stats.st_mtime > _file_mod_time)
  {
    _file_mod_time = stats.st_mtime;

    _initialized = false;
    _es.reset();
    _es2.reset();

    SolutionUserObject::initialSetup();
  }

  // Update time interpolation for ExodusII solution
  if (_file_type == 1 && _interpolate_times)
    updateExodusTimeInterpolation(getActualTime());
}

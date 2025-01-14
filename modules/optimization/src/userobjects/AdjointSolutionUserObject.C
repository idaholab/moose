//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdjointSolutionUserObject.h"

#include "libmesh/mesh_function.h"
#include "libmesh/exodusII_io.h"

#include <sys/stat.h>

registerMooseObject("OptimizationApp", AdjointSolutionUserObject);

InputParameters
AdjointSolutionUserObject::validParams()
{
  InputParameters params = SolutionUserObjectBase::validParams();
  params.addClassDescription(
      "Reads a variable from a mesh in one simulation to another specifically for loading forward "
      "solution in adjoint simulation during inverse optimization.");

  params.addRequiredParam<Real>(
      "reverse_time_end",
      "End time used for reversing the time integration when evaluating function derivative.");

  // Suppress some parameters that are irrelevant (i.e. users should use SolutionUserObject instead)
  params.suppressParameter<FileName>("es");
  params.suppressParameter<std::string>("system");
  params.suppressParameter<std::string>("timestep");

  return params;
}

AdjointSolutionUserObject::AdjointSolutionUserObject(const InputParameters & parameters)
  : SolutionUserObjectBase(parameters),
    _reverse_time_end(getParam<Real>("reverse_time_end")),
    _file_mod_time(std::numeric_limits<std::time_t>::min())
{
  if (!MooseUtils::hasExtension(_mesh_file, "e", /*strip_exodus_ext =*/true))
    paramError("mesh",
               "Performing transient adjoint simulation currently only works if the forward "
               "solution is written and read from exodus format.");
}

Real
AdjointSolutionUserObject::solutionSampleTime()
{
  return _reverse_time_end - _t + _dt;
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

    // Tell the parent class that we do need to re-initialize
    _initialized = false;
    // EquationSystems doesn't like the destructor that's called when there is a
    // unique_ptr::operator=
    _es.reset();
    _es2.reset();

    // Read the exodus file
    SolutionUserObjectBase::initialSetup();

    // Make sure to communicate what solution was actually loaded
    _interpolation_time = -_dt;
  }

  SolutionUserObjectBase::timestepSetup();
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CSGAction.h"

registerMooseAction("MooseApp", CSGAction, "no_action");

InputParameters
CSGAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Controls the generation of the Constructive Solid Geometry (CSG) "
                             "representation of the mesh generators in the input file.");
  params.addParam<bool>(
      "csg_only",
      true,
      "Whether to generate and output the CSG representation of the mesh generators instead of "
      "the finite element mesh, then exit. Ignored when --mesh-only, --split-mesh, or "
      "--parse-neml2-only is passed on the command line.");
  // A plain string rather than a FileName so that the path is relative to the working directory,
  // consistent with the --csg-only <path> command line argument
  params.addParam<std::string>("output_file",
                               "Name of the CSG JSON output file (Default: "
                               "\"<file_base>_csg.json\"). Overridden by --csg-only <path>.");
  params.addParam<bool>(
      "expand_units",
      false,
      "Whether to expand all engineering units into basic CSG components before output.");
  return params;
}

CSGAction::CSGAction(const InputParameters & params) : Action(params) {}

void
CSGAction::act()
{
}

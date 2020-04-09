//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "Tecplot.h"
#include "MooseApp.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/tecplot_io.h"

registerMooseObject("MooseApp", Tecplot);

InputParameters
Tecplot::validParams()
{
  // Get the base class parameters
  InputParameters params = OversampleOutput::validParams();

  // Add binary toggle
  params.addParam<bool>("binary", false, "Set Tecplot files to output in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  // Add optional parameter to turn on appending to ASCII files
  params.addParam<bool>(
      "ascii_append",
      false,
      "If true, append to an existing ASCII file rather than creating a new file each time");

  // Need a layer of geometric ghosting for mesh serialization
  params.addRelationshipManager("MooseGhostPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  // Add description for the Tecplot class
  params.addClassDescription("Object for outputting data in the Tecplot format");

  // Return the InputParameters
  return params;
}

Tecplot::Tecplot(const InputParameters & parameters)
  : OversampleOutput(parameters),
    _binary(getParam<bool>("binary")),
    _ascii_append(getParam<bool>("ascii_append")),
    _first_time(declareRestartableData<bool>("first_time", true))
{
#ifndef LIBMESH_HAVE_TECPLOT_API
  if (_binary)
  {
    mooseWarning(
        "Teclplot binary output requested but not available, outputting ASCII format instead.");
    _binary = false;
  }
#endif
}

void
Tecplot::output(const ExecFlagType & /*type*/)
{
  TecplotIO out(*_mesh_ptr, _binary, time() + _app.getGlobalTimeOffset());

  // Only set the append flag on the TecplotIO object if the user has
  // asked for it, and this is not the first time we called output().
  if (_ascii_append && !_first_time)
    out.ascii_append() = true;

  out.write_equation_systems(filename(), *_es_ptr);

  // If we're not appending, increment the file number.  If we are appending,
  // we'll use the same filename each time.
  if (_binary || !_ascii_append)
    _file_num++;

  // If this was the first time we called output(), the next time will not be
  // the first time.
  if (_first_time)
    _first_time = false;
}

std::string
Tecplot::filename()
{
  std::ostringstream output;
  output << _file_base;

  // If not appending, put the padded time step in the filename.
  if (_binary || !_ascii_append)
    output << "_" << std::setw(_padding) << std::setprecision(0) << std::setfill('0') << std::right
           << _file_num;

  // .plt extension is for binary files
  // .dat extension is for ASCII files
  if (_binary)
    return output.str() + ".plt";
  else
    return output.str() + ".dat";
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "GMVOutput.h"

#include "libmesh/equation_systems.h"
#include "libmesh/gmv_io.h"

registerMooseObjectAliased("MooseApp", GMVOutput, "GMV");

InputParameters
GMVOutput::validParams()
{
  // Get the base class parameters
  InputParameters params = OversampleOutput::validParams();

  // Advanced file options
  params.addParam<bool>("binary", true, "Output the file in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  // Add description for the GMVOutput class
  params.addClassDescription("Object for outputting data in the GMV format");

  // Need a layer of geometric ghosting for mesh serialization
  params.addRelationshipManager("MooseGhostPointNeighbors",
                                Moose::RelationshipManagerType::GEOMETRIC);

  // Return the InputParameters
  return params;
}

GMVOutput::GMVOutput(const InputParameters & parameters)
  : OversampleOutput(parameters), _binary(getParam<bool>("binary"))
{
}

void
GMVOutput::output(const ExecFlagType & /*type*/)
{
  GMVIO out(_es_ptr->get_mesh());
  out.write_equation_systems(filename(), *_es_ptr);
  _file_num++;
}

std::string
GMVOutput::filename()
{
  // Append the padded time step to the file base
  std::ostringstream output;
  output << _file_base << "_" << std::setw(_padding) << std::setprecision(0) << std::setfill('0')
         << std::right << _file_num;
  return output.str() + ".gmv";
}

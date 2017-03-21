/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// Moose includes
#include "GMVOutput.h"

// libMesh includes
#include "libmesh/equation_systems.h"
#include "libmesh/gmv_io.h"

template <>
InputParameters
validParams<GMVOutput>()
{
  // Get the base class parameters
  InputParameters params = validParams<BasicOutput<OversampleOutput>>();

  // Advanced file options
  params.addParam<bool>("binary", true, "Output the file in binary format");
  params.addParamNamesToGroup("binary", "Advanced");

  // Add description for the GMVOutput class
  params.addClassDescription("Object for outputting data in the GMV format");

  // Return the InputParameters
  return params;
}

GMVOutput::GMVOutput(const InputParameters & parameters)
  : BasicOutput<OversampleOutput>(parameters), _binary(getParam<bool>("binary"))
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

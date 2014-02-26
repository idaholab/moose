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
#include "GMVOutputter.h"

// libMesh includes
#include "libmesh/gmv_io.h"

template<>
InputParameters validParams<GMVOutputter>()
{
  // Get the base class parameters

  InputParameters params = validParams<OversampleOutputter>();
  params += validParams<FileOutputInterface>();

  // Supress un-available parameters
  params.suppressParameter<bool>("output_scalar_variables");
  params.suppressParameter<bool>("output_postprocessors");
  params.suppressParameter<bool>("scalar_as_nodal");
  params.suppressParameter<bool>("sequence");

  // Advanced file options
  params.addParam<unsigned int>("padding", 4, "The number of digits for the file extension (e.g., out_002.xda");
  params.addParam<bool>("binary", true, "Output the file in binary format");
  params.addParamNamesToGroup("padding binary", "Advanced");

  // Add description for the GMVOutputter class
  params.addClassDescription("Object for outputting data in the GMVOutputter format");

  // Return the InputParameters
  return params;
}

GMVOutputter::GMVOutputter(const std::string & name, InputParameters parameters) :
    OversampleOutputter(name, parameters),
    FileOutputInterface(name, parameters),
    _padding(getParam<unsigned int>("padding")),
    _binary(getParam<bool>("binary"))
{
  // Force sequence output
  /* Note: This does not change the behavior for this object b/c outputSetup() is empty, but it is
   * place here for consistency */
  sequence(true);
}

void
GMVOutputter::output()
{
  GMVIO out(_es_ptr->get_mesh());
  out.write_equation_systems(filename(), *_es_ptr);
}

void
GMVOutputter::outputNodalVariables()
{
  mooseError("Individual output of nodal variables is not support for GMV output");
}

void
GMVOutputter::outputElementalVariables()
{
  mooseError("Individual output of elemental variables is not support for GMV output");
}

void
GMVOutputter::outputPostprocessors()
{
  mooseError("Individual output of postprocessors is not support for GMV output");
}

void
GMVOutputter::outputScalarVariables()
{
  mooseError("Individual output of scalars is not support for GMV output");
}

std::string
GMVOutputter::filename()
{
  // Append the padded time step to the file base
  std::ostringstream output;
  output << _file_base
         << "_"
         << std::setw(_padding)
         << std::setprecision(0)
         << std::setfill('0')
         << std::right
         << _t_step;
  return output.str() + ".gmv";
}

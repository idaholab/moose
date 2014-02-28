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
#include "FileOutputInterface.h"

template<>
InputParameters validParams<FileOutputInterface>()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>("file_base", "The desired solution output name without an extension (Defaults appends '_out' to the input file name)");

  // Add the padding option and list it as 'Advanced'
  params.addParam<unsigned int>("padding", 4, "The number of for extension suffix (e.g., out.e-s002)");
  params.addParamNamesToGroup("padding", "Advanced");

  return params;
}

FileOutputInterface::FileOutputInterface(const std::string & /*name*/, InputParameters & parameters) :
    _file_base(parameters.get<std::string>("file_base")),
    _padding(parameters.get<unsigned int>("padding"))
{
}

FileOutputInterface::~FileOutputInterface()
{
}

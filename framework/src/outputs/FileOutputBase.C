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
#include "FileOutputBase.h"

template<>
InputParameters validParams<FileOutputBase>()
{
  // Create InputParameters object for this stand-alone object
  InputParameters params = emptyInputParameters();
  params.addParam<OutFileBase>("file_base", "The desired solution output name without an extension (Defaults appends '_out' to the input file name)");
  return params;
}

FileOutputBase::FileOutputBase(const std::string & name, InputParameters & parameters) :
    _file_base(parameters.get<OutFileBase>("file_base"))
{
}

FileOutputBase::~FileOutputBase()
{
}

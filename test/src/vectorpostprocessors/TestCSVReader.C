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

#include "TestCSVReader.h"

template <>
InputParameters
validParams<TestCSVReader>()
{
  InputParameters params = validParams<GeneralUserObject>();

  params.addRequiredParam<VectorPostprocessorName>("vectorpostprocessor",
                                                   "The vector posptorcessor to examine.");
  params.addRequiredParam<std::string>("vector",
                                       "The vector to consider from the VectorPosptorcessor.");
  params.addRequiredParam<processor_id_type>("rank", "The CPU rank to compare.");
  params.addRequiredParam<std::vector<double>>("gold", "The data to compare against.");

  return params;
}

TestCSVReader::TestCSVReader(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _vpp_data(getVectorPostprocessorValue("vectorpostprocessor", getParam<std::string>("vector"))),
    _rank(getParam<processor_id_type>("rank")),
    _gold(getParam<std::vector<double>>("gold"))
{
}

void
TestCSVReader::execute()
{
  if (_communicator.rank() == _rank)
    if (_gold != _vpp_data)
      mooseError("The supplied gold data does not match the VPP data on the given rank.");
}

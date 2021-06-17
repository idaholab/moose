//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorPointSource.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", VectorPostprocessorPointSource);

defineLegacyParams(VectorPostprocessorPointSource);

InputParameters
VectorPostprocessorPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addClassDescription("Apply a point load defined by VectorPostprocessor.");

  params.addParam<VectorPostprocessorName>(
      "vector_postprocessor",
      "The name of the VectorPostprocessor containing positions and corresponding load values");
  params.addParam<std::string>("x_coord_name", "x", "name of column containing x coordinates.");
  params.addParam<std::string>("y_coord_name", "y", "name of column containing y coordinates.");
  params.addParam<std::string>("z_coord_name", "z", "name of column containing z coordinates.");
  params.addParam<std::string>("value_name", "value", "name of column containing values.");
  return params;
}

VectorPostprocessorPointSource::VectorPostprocessorPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _vpp_values(getVectorPostprocessorValue(
        "vector_postprocessor", getParam<std::string>("value_name"), true)),
    _x_coord(getVectorPostprocessorValue(
        "vector_postprocessor", getParam<std::string>("x_coord_name"), true)),
    _y_coord(getVectorPostprocessorValue(
        "vector_postprocessor", getParam<std::string>("y_coord_name"), true)),
    _z_coord(getVectorPostprocessorValue(
        "vector_postprocessor", getParam<std::string>("z_coord_name"), true))
{
}

void
VectorPostprocessorPointSource::addPoints()
{
  mooseAssert(_vpp_values.size() * _x_coord.size() * _y_coord.size() * _z_coord.size() != 0,
              "VectorPostprocessorPointSource::addPoints():  Nothing read from vpp, \n"
              "vpp must have data before the Dirac Kernel is called.\n"
              "Try setting \"execute_on = timestep_begin\" in the vpp being read. ");

  _point_to_index.clear();
  for (std::size_t i = 0; i < _x_coord.size(); ++i)
  {
    Point pt(_x_coord[i], _y_coord[i], _z_coord[i]);
    _point_to_index[pt] = i;
    addPoint(pt, i);
  }
}

Real
VectorPostprocessorPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _vpp_values[_point_to_index[_current_point]];
}

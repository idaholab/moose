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

  params.addParam<bool>(
      "use_broadcast",
      false,
      "Causes this diracKernel to use a broadcasted version of the vector instead "
      "of a scattered version of the vector (the default).  This is slower - but "
      "is useful for debugging and testing");

  return params;
}

VectorPostprocessorPointSource::VectorPostprocessorPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _use_broadcast(getParam<bool>("use_broadcast")),
    _vpp_values(getVectorPostprocessorValue(
        "vector_postprocessor", getParam<std::string>("value_name"), _use_broadcast)),
    _x_coord_name(getParam<std::string>("x_coord_name"), _use_broadcast),
    _y_coord_name(getParam<std::string>("y_coord_name"), _use_broadcast),
    _z_coord_name(getParam<std::string>("z_coord_name"), _use_broadcast)
{
}

void
VectorPostprocessorPointSource::addPoints()
{
  const auto xcoord =
      getVectorPostprocessorValue("vector_postprocessor", _x_coord_name, _use_broadcast);
  const auto ycoord =
      getVectorPostprocessorValue("vector_postprocessor", _y_coord_name, _use_broadcast);
  const auto zcoord =
      getVectorPostprocessorValue("vector_postprocessor", _z_coord_name, _use_broadcast);

  mooseAssert(_vpp_values.size() * xcoord.size() * ycoord.size() * zcoord.size() != 0,
              "VectorPostprocessorPointSource::addPoints():  Nothing read from vpp, \n"
              "vpp must have data before the Dirac Kernel is called.\n"
              "Try setting \"execute_on = timestep_begin\" in the vpp being read. ");

  for (std::size_t i = 0; i < xcoord.size(); ++i)
  {
    Point pt(Point(xcoord[i], ycoord[i], zcoord[i]));
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

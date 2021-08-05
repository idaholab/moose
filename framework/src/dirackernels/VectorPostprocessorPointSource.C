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

  params.addClassDescription("Apply a point load defined by Reporter or VectorPostprocessor.");

  params.addParam<std::string>(
      "vector_postprocessor",
      "The name of the VectorPostprocessor containing positions and corresponding load values.  "
      "This will be appended onto the reporter name as <vector_postprocessor>/<vector_name>.");
  params.addParam<std::string>(
      "x_coord_name",
      "x",
      "<reporter>/<x_coord_name> containing x coordinates.  This is just "
      "the x_coord_name if providing input for a vector_postprocessor name.");
  params.addParam<std::string>(
      "y_coord_name",
      "y",
      "<reporter>/<y_coord_name> containing y coordinates.  This is just "
      "the y_coord_name if providing input for a vector_postprocessor name.");
  params.addParam<std::string>(
      "z_coord_name",
      "z",
      "<reporter>/<z_coord_name> containing z coordinates.  This is just "
      "the z_coord_name if providing input for a vector_postprocessor name.");
  params.addParam<std::string>("value_name",
                               "value",
                               "<reporter>/<value_name> containing values.  This is just the "
                               "value_name if providing input for a vector_postprocessor name.");

  return params;
}

VectorPostprocessorPointSource::VectorPostprocessorPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _vpp_values(
        isParamValid("vector_postprocessor")
            ? getReporterValueByName<std::vector<Real>>(
                  getParam<std::string>("vector_postprocessor") + "/" +
                  getParam<std::string>("value_name"))
            : getReporterValueByName<std::vector<Real>>(getParam<std::string>("value_name"))),
    _x_coord(
        isParamValid("vector_postprocessor")
            ? getReporterValueByName<std::vector<Real>>(
                  getParam<std::string>("vector_postprocessor") + "/" +
                  getParam<std::string>("x_coord_name"))
            : getReporterValueByName<std::vector<Real>>(getParam<std::string>("x_coord_name"))),
    _y_coord(
        isParamValid("vector_postprocessor")
            ? getReporterValueByName<std::vector<Real>>(
                  getParam<std::string>("vector_postprocessor") + "/" +
                  getParam<std::string>("y_coord_name"))
            : getReporterValueByName<std::vector<Real>>(getParam<std::string>("y_coord_name"))),
    _z_coord(isParamValid("vector_postprocessor")
                 ? getReporterValueByName<std::vector<Real>>(
                       getParam<std::string>("vector_postprocessor") + "/" +
                       getParam<std::string>("z_coord_name"))
                 : getReporterValueByName<std::vector<Real>>(getParam<std::string>("z_coord_name")))
{
}

void
VectorPostprocessorPointSource::addPoints()
{
  if (_vpp_values.size() != _x_coord.size() || _vpp_values.size() != _y_coord.size() ||
      _vpp_values.size() != _z_coord.size())
  {
    std::string errMsg = "The value and coordinate vectors are a different size.  \n"
                         "There must be one value per coordinate.  If the sizes are \n"
                         "zero, the reporter or vpp may not have been initialized with data \n"
                         "before the Dirac Kernel is called.  \n"
                         "Try setting \"execute_on = timestep_begin\" in the vpp being read. \n"
                         "value size = " +
                         std::to_string(_vpp_values.size()) +
                         ";  x_coord size = " + std::to_string(_x_coord.size()) +
                         ";  y_coord size = " + std::to_string(_y_coord.size()) +
                         ";  z_coord size = " + std::to_string(_z_coord.size());

    mooseError(errMsg);
  }

  _point_to_index.clear();
  for (std::size_t i = 0; i < _vpp_values.size(); ++i)
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

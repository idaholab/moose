//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantPointSource.h"
#include "MooseUtils.h"
#include "DelimitedFileReader.h"

registerMooseObject("MooseApp", ConstantPointSource);

defineLegacyParams(ConstantPointSource);

InputParameters
ConstantPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addParam<std::vector<Real>>("value", "The list of values (v0...vn) of the point sources");
  params.addParam<std::vector<Real>>("point",
                                     "The list of coordinates (x0,y0,z0...xn,yn,zn) of the points");
  params.declareControllable("value");
  params.addParam<FileName>("position_value_file",
                            "The file containing positions and corresponding values.");
  return params;
}

ConstantPointSource::ConstantPointSource(const InputParameters & parameters)
  : DiracKernel(parameters)
{

  std::vector<Real> value = getParam<std::vector<Real>>("value");
  std::vector<Real> point_param = getParam<std::vector<Real>>("point");

  if (isParamValid("position_value_file"))
  {
    readFromFile(value, point_param);
  }

  if (value.size() == 0 || point_param.size() == 0)
    mooseError("ConstantPointSource: No values or points created by input or position_value_file.");

  size_t dim = point_param.size() / value.size();
  Point pt;
  _point_to_value.clear();
  for (size_t i = 0; i < value.size(); ++i)
  {
    pt(0) = point_param[dim * i + 0];
    if (dim > 1)
    {
      pt(1) = point_param[dim * i + 1];
      if (dim > 2)
      {
        pt(2) = point_param[dim * i + 2];
      }
    }
    _point_to_value[pt] = value[i];
  }
}

void
ConstantPointSource::addPoints()
{
  unsigned index = 0;
  for (auto & point : _point_to_value)
  {
    addPoint(point.first);
    ++index;
  }
}

Real
ConstantPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _point_to_value[_current_point];
}

void
ConstantPointSource::readFromFile(std::vector<Real> & value, std::vector<Real> & point_param)
{
  if (value.size() != 0)
    mooseError("ConstantPointSource: Please provide value and point in the input file or read "
               "them from a position_value_file, not both.");

  MooseUtils::DelimitedFileReader position_value_file(getParam<FileName>("position_value_file"));
  position_value_file.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::AUTO);
  position_value_file.read();
  std::vector<std::vector<Real>> data = position_value_file.getData();
  size_t num_columns = data.size();
  if (num_columns != 4 && num_columns != 5)
    mooseError("ConstantPointSource: The number of columns in ",
               getParam<FileName>("position_value_file"),
               " should be 4, or if you are including the nodal id then 5.",
               " Even 1d and 2d models require all 3 coordinates (x,y,z)",
               " when reading from file.");

  unsigned int skip_gid_column = 0;
  if (num_columns == 5)
    skip_gid_column = 1;

  unsigned int num_rows = data[0].size();
  point_param.resize(3 * num_rows);
  value.resize(num_rows);

  for (unsigned int i = 0; i < num_rows; ++i)
  {
    point_param[3 * i + 0] = data[0][i];
    point_param[3 * i + 1] = data[1][i];
    point_param[3 * i + 2] = data[2][i];
    value[i] = data[3 + skip_gid_column][i];
  }
}

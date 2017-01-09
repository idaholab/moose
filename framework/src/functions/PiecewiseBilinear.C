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

#include "PiecewiseBilinear.h"
#include "ColumnMajorMatrix.h"
#include "BilinearInterpolation.h"

template<>
InputParameters validParams<PiecewiseBilinear>()
{
  InputParameters params = validParams<Function>();
  params.addParam<FileName>("data_file", "", "File holding csv data for use with PiecewiseBilinear");
  params.addParam<std::vector<Real> >("x", "The x abscissa values");
  params.addParam<std::vector<Real> >("y", "The y abscissa values");
  params.addParam<std::vector<Real> >("z", "The ordinate values");
  params.addParam<int>("axis", -1, "The axis used (0, 1, or 2 for x, y, or z).");
  params.addParam<int>("xaxis", -1, "The coordinate used for x-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<int>("yaxis", -1, "The coordinate used for y-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the axis, yaxis, or xaxis values");
  params.addParam<bool>("radial", false, "Set to true if you want to interpolate along a radius rather that along a specific axis, and note that you have to define xaxis and yaxis in the input file");
  return params;
}

PiecewiseBilinear::PiecewiseBilinear(const InputParameters & parameters) :
    Function(parameters),
    _data_file_name(getParam<FileName>("data_file")),
    _axis(getParam<int>("axis")),
    _yaxis(getParam<int>("yaxis")),
    _xaxis(getParam<int>("xaxis")),
    _axisValid( _axis > -1 && _axis < 3 ),
    _yaxisValid( _yaxis > -1 && _yaxis < 3 ),
    _xaxisValid( _xaxis > -1 && _xaxis < 3 ),
    _scale_factor( getParam<Real>("scale_factor") ),
    _radial(getParam<bool>("radial"))
{

  if (!_axisValid && !_yaxisValid && !_xaxisValid)
    mooseError("In PiecewiseBilinear " << _name << ": None of axis, yaxis, or xaxis properly defined.  Allowable range is 0-2");

  if (_axisValid && (_yaxisValid || _xaxisValid))
    mooseError("In PiecewiseBilinear " << _name << ": Cannot define axis with either yaxis or xaxis");

  if (_radial && (!_yaxisValid || !_xaxisValid))
    mooseError("In PiecewiseBilinear " << _name << ": yaxis and xaxis must be defined when radial = true");

  std::vector<Real> x;
  std::vector<Real> y;
  ColumnMajorMatrix z;
  std::vector<Real> z_vec;

  if (!_data_file_name.empty())
  {
    if ( parameters.isParamValid("x") || parameters.isParamValid("y") || parameters.isParamValid("z") )
      mooseError("In PiecewiseBilinear: Cannot specify 'data_file' and 'x', 'y', or 'z' together.");
    else
      parse( x, y, z );
  }

  else if ( !(parameters.isParamValid("x") && parameters.isParamValid("y") && parameters.isParamValid("z")) )
      mooseError("In PiecewiseBilinear: 'x' and 'y' and 'z' must be specified if any one is specified.");

  else
  {
    x = getParam<std::vector<Real> >("x");
    y = getParam<std::vector<Real> >("y");
    z_vec = getParam<std::vector<Real> >("z");

    //check that size of z = (size of x)*(size of y)
    if (z_vec.size() != x.size()*y.size())
      mooseError("In PiecewiseBilinear: Size of z should be the size of x times the size of y.");

    //reshape and populate z matrix
    z.reshape(y.size(),x.size());
    int idx = 0;
    for (unsigned int i = 0; i < y.size(); i++)
      for (unsigned int j = 0; j < x.size(); j++)
      {
        z(i,j) = z_vec[idx];
        idx += 1;
      }
  }

  _bilinear_interp = libmesh_make_unique<BilinearInterpolation>(x, y, z);
}

PiecewiseBilinear::~PiecewiseBilinear()
{
}

Real
PiecewiseBilinear::value(Real t, const Point & p)
{
  Real retVal(0);
  if (_yaxisValid && _xaxisValid && _radial)
  {
    Real rx = p(_xaxis)*p(_xaxis);
    Real ry = p(_yaxis)*p(_yaxis);
    Real r = std::sqrt(rx + ry);
    retVal = _bilinear_interp->sample( r, t );
  }
  else if (_axisValid)
    retVal = _bilinear_interp->sample( p(_axis), t );
  else if (_yaxisValid && !_radial)
  {
    if (_xaxisValid)
      retVal = _bilinear_interp->sample( p(_xaxis), p(_yaxis) );
    else
      retVal = _bilinear_interp->sample( t, p(_yaxis) );
  }
  else
    retVal = _bilinear_interp->sample( p(_xaxis), t );

  return retVal * _scale_factor;
}

void
PiecewiseBilinear::parse( std::vector<Real> & x,
                          std::vector<Real> & y,
                          ColumnMajorMatrix & z)
{
  std::ifstream file(_data_file_name.c_str());
  if (!file.good())
    mooseError("In PiecewiseBilinear " << _name << ": Error opening file '" + _data_file_name + "'.");
  std::string line;
  unsigned int linenum= 0;
  unsigned int itemnum = 0;
  unsigned int num_cols = 0;
  std::vector<Real> data;

  while (getline (file, line))
  {
    linenum++;
    std::istringstream linestream(line);
    std::string item;
    itemnum = 0;

    while (getline (linestream, item, ','))
    {
      itemnum++;
      std::istringstream i(item);
      Real d;
      i >> d;
      data.push_back(d);
    }

    if (linenum == 1)
      num_cols = itemnum;
    else if (num_cols+1 != itemnum)
      mooseError("In PiecewiseBilinear " << _name << ": Read " << itemnum << " columns of data but expected " << num_cols+1
                 << " columns while reading line " << linenum << " of '" << _data_file_name << "'.");
  }

  x.resize(itemnum-1);
  y.resize(linenum-1);
  z.reshape(linenum-1,itemnum-1);
  unsigned int offset(0);
  // Extract the first line's data (the x axis data)
  for (unsigned int j = 0; j < itemnum-1; ++j)
  {
    x[j] = data[offset];
    ++offset;
  }
  for (unsigned int i = 0; i < linenum-1; ++i)
  {
    // Extract the y axis entry for this line
    y[i] = data[offset];
    ++offset;

    // Extract the function values for this row in the matrix
    for (unsigned int j = 0; j < itemnum-1; ++j)
    {
      z(i,j) = data[offset];
      ++offset;
    }
  }

  if (data.size() != offset)
    mooseError("ERROR! Inconsistency in data read from '" + _data_file_name + "' for PiecewiseBilinear function.");
}

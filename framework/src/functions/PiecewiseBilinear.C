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

/* PiecewiseBilinear reads from a file the information necessary to build the vectors x and y and
 * the ColumnMajorMatrix z, and then sends those (along with a sample point) to BilinearInterpolation.
 * See BilinearInterpolation in moose/src/utils for a description of how that works...it is a 2D linear
 * interpolation algorithm.  The format of the data file must be the following:
 *
 * 1,2
 * 1,1,2
 * 2,3,4
 *
 * The first row is the x vector data.
 * After the first row, the first column is the y vector data.
 * The rest of the data is used to build the ColumnMajorMatrix z.
 *
 * x = [1 2]
 * y = [1 2]
 *
 * z = [1 2]
 *     [3 4]
 *
 *     z has to be x.size() by y.size()
 *
 * PiecewisBilinear also sends samples to BilinearInterpolation.  These samples are the z-coordinate of the current
 * integration point, and the current value of time.  The name of the file that contains this data has to be included
 * in the function block of the inpute file like this...yourFileName = example.csv.
 */

#include "PiecewiseBilinear.h"

template<>
InputParameters validParams<PiecewiseBilinear>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("yourFileName", "File holding your csv data for use with PiecewiseBilinear");
  params.addParam<int>("axis", -1, "The axis used (0, 1, or 2 for x, y, or z).");
  params.addParam<int>("xaxis", -1, "The coordinate used for x-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<int>("yaxis", -1, "The coordinate used for y-axis data (0, 1, or 2 for x, y, or z).");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the axis, yaxis, or xaxis values");
  return params;
}

PiecewiseBilinear::PiecewiseBilinear(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _bilinear_interp( NULL ),
  _file_name( getParam<std::string>("yourFileName") ),
  _axis(getParam<int>("axis")),
  _yaxis(getParam<int>("yaxis")),
  _xaxis(getParam<int>("xaxis")),
  _axisValid( _axis > -1 && _axis < 3 ),
  _yaxisValid( _yaxis > -1 && _yaxis < 3 ),
  _xaxisValid( _xaxis > -1 && _xaxis < 3 ),
  _scale_factor( getParam<Real>("scale_factor") )
{
  if (!_axisValid && !_yaxisValid && !_xaxisValid)
  {
    mooseError("Error in " << _name << ". None of axis, yaxis, or xaxis properly defined.  Allowable range is 0-2");
  }
  if (_axisValid && (_yaxisValid || _xaxisValid))
  {
    mooseError("Error in " << _name << ". Cannot define axis with either yaxis or xaxis");
  }

  std::vector<Real> x;
  std::vector<Real> y;
  ColumnMajorMatrix z;

  // Parse to get x, y, z
  parse( x, y, z );

  _bilinear_interp = new BilinearInterpolation( x, y, z );
}

PiecewiseBilinear::~PiecewiseBilinear()
{
  delete _bilinear_interp;
}

Real
PiecewiseBilinear::value( Real t, const Point & p)
{
  Real retVal(0);
  if (_axisValid)
  {
    retVal = _bilinear_interp->sample( p(_axis), t );
  }
  else if (_yaxisValid)
  {
    if (_xaxisValid)
    {
      retVal = _bilinear_interp->sample( p(_xaxis), p(_yaxis) );
    }
    else
    {
      retVal = _bilinear_interp->sample( t, p(_yaxis) );
    }
  }
  else
  {
    retVal = _bilinear_interp->sample( p(_xaxis), t );
  }

  return retVal * _scale_factor;
}

void
PiecewiseBilinear::parse( std::vector<Real> & x,
                          std::vector<Real> & y,
                          ColumnMajorMatrix & z)
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from PiecewiseBilinear function.");
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
       data.push_back( d );
     }
     if (linenum == 1)
     {
       num_cols = itemnum;
     }
     else
     {
       if (num_cols+1 != itemnum)
       {
         mooseError("ERROR! Read "<<itemnum<<" columns of data but expected "<<num_cols+1<<
                    " columns while reading line "<<linenum<<" of '"+ _file_name + "' for PiecewiseBilinear function.");
       }
     }
   }

   x.resize(itemnum-1);
   y.resize(linenum-1);
   z.reshape(linenum-1,itemnum-1);
   unsigned int offset(0);
   // Extract the first line's data (the x axis data)
   for (unsigned int j(0); j < itemnum-1; ++j)
   {
     x[j] = data[offset];
     ++offset;
   }
   for (unsigned int i(0); i < linenum-1; ++i)
   {
     // Extract the y axis entry for this line
     y[i] = data[offset];
     ++offset;

     // Extract the function values for this row in the matrix
     for (unsigned int j(0); j < itemnum-1; ++j)
     {
       z(i,j) = data[offset];
       ++offset;
     }
   }
   if (data.size() != offset)
   {
     mooseError("ERROR! Inconsistency in data read from '" + _file_name + "' for PiecewiseBilinear function.");
   }
}

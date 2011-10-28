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

/*This works just like PiecewiseLinear except the x and y vectors are defined in a .csv file instead of the input file
 *your file should have the following form
 *
 *0,1,2,3,4
 *0,1,2,3,4
 *
 *where the first row is the x-axis and the second row is the y-axis
 *
 *As an example, the x-axis could represent time and the y-axis could be pressure amplitude
 *
 **/

#include "PiecewiseLinearFile.h"
#include "Moose.h"

template<>
InputParameters validParams<PiecewiseLinearFile>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("yourFileName", "File holding your csv data for use with PiecewiseLilinearFile");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the axis values");
  return params;
}

PiecewiseLinearFile::PiecewiseLinearFile(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _linear_interp( NULL ),
  _file_name( getParam<std::string>("yourFileName") ),
  _scale_factor( getParam<Real>("scale_factor") )
{
  std::vector<Real> x;
  std::vector<Real> y;

  // Parse to get x, y
  parse( x, y );

  _linear_interp = new LinearInterpolation( x, y );
}

PiecewiseLinearFile::~PiecewiseLinearFile()
{
  delete _linear_interp;
}

Real
PiecewiseLinearFile::value(Real t, const Point &)
{
  return  _linear_interp->sample( t );
}

void
PiecewiseLinearFile::parse( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from PiecewiseLinearFile function.");
  std::string line;
  unsigned int linenum = 0;
  unsigned int itemnum;
  std::vector<Real> data;

  while (getline (file, line))
  {
    linenum++;
//     std::cout << "\nLine #" << linenum << ":" << std::endl;
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
      //       std::cout << "Item #" << itemnum << ": " << item << std::endl;

    }
  }
//   std::cout << " linenum " << linenum << std::endl;
//   std::cout << " itemnum " << itemnum << std::endl;

  x.resize(itemnum);
  y.resize(itemnum);
  unsigned int offset(0);
  // Extract the first line's data (the x axis data)
  for (unsigned int i(0); i < itemnum; ++i)
  {
    x[i] = data[offset];
    ++offset;
/*    std::cout << " " << std::endl;
    std::cout << " x " << x[i] << std::endl;
    std::cout << " i " << i << std::endl;
    std::cout << " " << std::endl;
*/
  }
  for (unsigned int j(0); j < itemnum; ++j)
  {
    // Extract the y axis entry for this line
    y[j] = data[offset];
    ++offset;
/*    std::cout << " " << std::endl;
    std::cout << " y " << y[j] << std::endl;
    std::cout << " j " << j << std::endl;
    std::cout << " " << std::endl;
*/
  }
}

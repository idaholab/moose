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
  params.addParam<std::string>("format", "rows" ,"Format of csv data file that is in either in columns or rows");
  return params;
}

PiecewiseLinearFile::PiecewiseLinearFile(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _linear_interp( NULL ),
  _file_name( getParam<std::string>("yourFileName") ),
  _scale_factor( getParam<Real>("scale_factor") ),
  _format( getParam<std::string>("format") )
{
  std::vector<Real> x;
  std::vector<Real> y;

  // Parse to get x, y
  if (_format.compare(0, 4, "rows")==0)
  {
    parseRows( x, y );
  }
  else if (_format.compare(0, 7, "columns")==0)
  {
    parseColumns( x, y);
  }
  else
  {
    mooseError("Invalid option for format: "+_format+" in PiecewiseLinearFile.  Valid options are rows and columns.");
  }


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

Real
PiecewiseLinearFile::integral()
{
  return _linear_interp->integrate();
}

Real
PiecewiseLinearFile::average()
{
  return integral()/(_linear_interp->domain(_linear_interp->getSampleSize()-1)-_linear_interp->domain(0));
}

bool parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec)
{
  std::string line;
  myvec.clear();
  bool gotline(false);
  if (getline(ifs,line))
  {
    gotline=true;

    //Replace all commas with spaces
    while(size_t pos=line.find(','))
    {
      if (pos == line.npos)
        break;
      line.replace(pos,1,1,' ');
    }

    //Harvest floats separated by whitespace
    std::istringstream iss(line);
    Real f;
    while (iss>>f)
    {
      myvec.push_back(f);
    }
  }
  return gotline;
}

void
PiecewiseLinearFile::parseRows( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from PiecewiseLinearFile function.");
  std::string line;

  while(parseNextLineReals(file, x))
  {
    if (x.size() >0)
      break;
  }

  if (x.size() == 0)
    mooseError("File '" + _file_name + "' contains no data for PiecewiseLinearFile function.");

  while(parseNextLineReals(file, y))
  {
    if (y.size() >0)
      break;
  }

  if (y.size() == 0)
    mooseError("File '" + _file_name + "' contains no y data for PiecewiseLinearFile function.");
  else if (y.size() != x.size())
    mooseError("Lengths of x and y data do not match in file '" + _file_name + "' for PiecewiseLinearFile function.");

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch)){
    if (scratch.size() > 0)
      mooseError("Read more than two rows of data from file '" + _file_name + "' for PiecewiseLinearFile function.  Did you mean to use \"format = columns\"?");
  }

}

void
PiecewiseLinearFile::parseColumns( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from PiecewiseLinearFile function.");
  std::string line;

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch))
  {
    if (scratch.size() > 0){
      if (scratch.size() != 2)
        mooseError("Read more than 2 columns of data from file '" + _file_name + "' for PiecewiseLinearFile function.  Did you mean to use \"format = rows\"?");
      x.push_back(scratch[0]);
      y.push_back(scratch[1]);
    }
  }
}

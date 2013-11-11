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

#include "Piecewise.h"

template<>
InputParameters validParams<Piecewise>()
{
  InputParameters params = validParams<Function>();
  params.addParam<std::vector<Real> >("x", "The abscissa values");
  params.addParam<std::vector<Real> >("y", "The ordinate values");
  params.addParam<std::string>("yourFileName", "File holding your csv data for use with Piecewise");
  params.addParam<std::string>("format", "rows" ,"Format of csv data file that is in either in columns or rows");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.addParam<int>("axis", "The axis used (0, 1, or 2 for x, y, or z) if this is to be a function of position");
  return params;
}

Piecewise::Piecewise(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _scale_factor( getParam<Real>("scale_factor") ),
  _linear_interp( NULL ),
  _has_axis(false),
  _file_name( isParamValid("yourFileName") ? getParam<std::string>("yourFileName") : "")
{
  std::vector<Real> x;
  std::vector<Real> y;

  if (parameters.isParamValid("yourFileName"))
  {
    if (parameters.isParamValid("x"))
    {
      mooseError("Error in Piecewise: Cannot specify yourFileName and either x or y.");
    }
    std::string format = getParam<std::string>("format");
    if (format.compare(0, 4, "rows")==0)
    {
      parseRows( x, y );
    }
    else if (format.compare(0, 7, "columns")==0)
    {
      parseColumns( x, y);
    }
    else
    {
      mooseError("Invalid option for format: "+format+" in "+_name+".  Valid options are rows and columns.");
    }
  }
  else if (parameters.isParamValid("x"))
  {
    if (!parameters.isParamValid("y"))
    {
      mooseError("Error in Piecewise: If one of x and y is specified, both must be.");
    }
    x = getParam<std::vector<Real> >("x");
    y = getParam<std::vector<Real> >("y");
  }
  else
  {
    mooseError("Error in Piecewise: Either yourFileName or both of x and y must be specified.");
  }

  _linear_interp = new LinearInterpolation( x, y );


  if (parameters.isParamValid("axis"))
  {
    _axis=parameters.get<int>("axis");
    if (_axis < 0 || _axis > 2)
      mooseError("In Piecewise function axis="<<_axis<<" outside allowable range (0-2).");
    _has_axis = true;
  }
}

Piecewise::~Piecewise()
{
  delete _linear_interp;
}

Real
Piecewise::functionSize()
{
  return _linear_interp->getSampleSize();
}

Real
Piecewise::domain(int i)
{
  return _linear_interp->domain(i);
}

Real
Piecewise::range(int i)
{
  return _linear_interp->range(i);
}

bool
Piecewise::parseNextLineReals(std::ifstream & ifs, std::vector<Real> &myvec)
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
Piecewise::parseRows( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from Piecewise function.");
  std::string line;

  while(parseNextLineReals(file, x))
  {
    if (x.size() >0)
      break;
  }

  if (x.size() == 0)
    mooseError("File '" + _file_name + "' contains no data for Piecewise function.");

  while(parseNextLineReals(file, y))
  {
    if (y.size() >0)
      break;
  }

  if (y.size() == 0)
    mooseError("File '" + _file_name + "' contains no y data for Piecewise function.");
  else if (y.size() != x.size())
    mooseError("Lengths of x and y data do not match in file '" + _file_name + "' for Piecewise function.");

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch)){
    if (scratch.size() > 0)
      mooseError("Read more than two rows of data from file '" + _file_name + "' for Piecewise function.  Did you mean to use \"format = columns\"?");
  }

}

void
Piecewise::parseColumns( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _file_name + "' from Piecewise function.");
  std::string line;

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch))
  {
    if (scratch.size() > 0){
      if (scratch.size() != 2)
        mooseError("Read more than 2 columns of data from file '" + _file_name + "' for Piecewise function.  Did you mean to use \"format = rows\"?");
      x.push_back(scratch[0]);
      y.push_back(scratch[1]);
    }
  }
}

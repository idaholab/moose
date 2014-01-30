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
  params.addParam<std::vector<Real> >("xy_data", "All function data, supplied in abscissa, ordinate pairs");
  params.addParam<std::vector<Real> >("x", "The abscissa values");
  params.addParam<std::vector<Real> >("y", "The ordinate values");
  params.addParam<std::string>("data_file", "File holding csv data for use with Piecewise");
  params.addParam<std::string>("yourFileName", "File holding your csv data for use with Piecewise (Deprecated)");
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
  _data_file_name( isParamValid("yourFileName") ? getParam<std::string>("yourFileName") :
              (isParamValid("data_file") ? getParam<std::string>("data_file") : ""))
{
  std::vector<Real> x;
  std::vector<Real> y;

  if (_data_file_name != "")
  {
    if (parameters.isParamValid("yourFileName"))
    {
      mooseWarning("In Piecewise, 'yourFileName' is Deprecated.  Use 'data_file' instead.");

      if (parameters.isParamValid("data_file"))
      {
        mooseError("In Piecewise, cannot specify both 'yourFileName' and 'data_file'.");
      }
    }

    if ((parameters.isParamValid("x")) ||
        (parameters.isParamValid("y")) ||
        (parameters.isParamValid("xy_data")))
    {
      mooseError("In Piecewise: Cannot specify 'data_file' and 'x', 'y', or 'xy_data' together.");
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
      mooseError("Invalid option for format: "+format+" in "+_name+".  Valid options are 'rows' and 'columns'.");
    }
  }
  else if ((parameters.isParamValid("x")) ||
           (parameters.isParamValid("y")))
  {
    if (! ((parameters.isParamValid("x")) &&
          (parameters.isParamValid("y"))))
    {
      mooseError("In Piecewise: Both 'x' and 'y' must be specified if either one is specified.");
    }
    if (parameters.isParamValid("xy_data"))
    {
      mooseError("In Piecewise: Cannot specify 'x', 'y', and 'xy_data' together.");
    }
    x = getParam<std::vector<Real> >("x");
    y = getParam<std::vector<Real> >("y");
  }
  else if (parameters.isParamValid("xy_data"))
  {
    std::vector<Real> xy = getParam<std::vector<Real> >("xy_data");
    unsigned int xy_size = xy.size();
    if (xy_size % 2 != 0)
    {
      mooseError("In Piecewise: Length of data provided in 'xy_data' must be a multiple of 2.");
    }
    unsigned int x_size = xy_size/2;
    x.reserve(x_size);
    y.reserve(x_size);
    for (unsigned int i=0; i<xy_size/2; ++i)
    {
      x.push_back(xy[i*2]);
      y.push_back(xy[i*2+1]);
    }
  }
  else
  {
    mooseError("In Piecewise: Either 'data_file', 'x' and 'y', or 'xy_data' must be specified.");
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
  std::ifstream file(_data_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _data_file_name + "' from Piecewise function.");
  std::string line;

  while(parseNextLineReals(file, x))
  {
    if (x.size() >0)
      break;
  }

  if (x.size() == 0)
    mooseError("File '" + _data_file_name + "' contains no data for Piecewise function.");

  while(parseNextLineReals(file, y))
  {
    if (y.size() >0)
      break;
  }

  if (y.size() == 0)
    mooseError("File '" + _data_file_name + "' contains no y data for Piecewise function.");
  else if (y.size() != x.size())
    mooseError("Lengths of x and y data do not match in file '" + _data_file_name + "' for Piecewise function.");

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch)){
    if (scratch.size() > 0)
      mooseError("Read more than two rows of data from file '" + _data_file_name + "' for Piecewise function.  Did you mean to use \"format = columns\"?");
  }

}

void
Piecewise::parseColumns( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_data_file_name.c_str());
  if (!file.good())
    mooseError("Error opening file '" + _data_file_name + "' from Piecewise function.");
  std::string line;

  std::vector<Real> scratch;
  while(parseNextLineReals(file, scratch))
  {
    if (scratch.size() > 0){
      if (scratch.size() != 2)
        mooseError("Read more than 2 columns of data from file '" + _data_file_name + "' for Piecewise function.  Did you mean to use \"format = rows\"?");
      x.push_back(scratch[0]);
      y.push_back(scratch[1]);
    }
  }
}

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
  params.addParam<FileName>("data_file", "", "File holding csv data for use with Piecewise");
  params.addParam<unsigned int>("x_index_in_file", 0, "The abscissa index in the data file");
  params.addParam<unsigned int>("y_index_in_file", 1, "The ordinate index in the data file");
  params.addParam<bool>("xy_in_file_only", true, "If the data file only contains abscissa and ordinate data");
  params.addParam<std::string>("format", "rows" ,"Format of csv data file that is in either in columns or rows");
  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  params.addParam<int>("axis", "The axis used (0, 1, or 2 for x, y, or z) if this is to be a function of position");
  return params;
}

Piecewise::Piecewise(const InputParameters & parameters) :
    Function(parameters),
    _scale_factor(getParam<Real>("scale_factor")),
    _has_axis(false),
    _data_file_name(getParam<FileName>("data_file")),
    _x_index(getParam<unsigned int>("x_index_in_file")),
    _y_index(getParam<unsigned int>("y_index_in_file")),
    _xy_only(getParam<bool>("xy_in_file_only"))
{
  std::vector<Real> x;
  std::vector<Real> y;

  if (_data_file_name != "")
  {
    if ((parameters.isParamValid("x")) ||
        (parameters.isParamValid("y")) ||
        (parameters.isParamValid("xy_data")))
    {
      mooseError("In Piecewise " << _name << ": Cannot specify 'data_file' and 'x', 'y', or 'xy_data' together.");
    }
    if (_x_index == _y_index)
      mooseError("In Piecewise " << _name << ": 'x_index_in_file' and 'y_index_in_file' are set to the same value.");
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
      mooseError("In Piecewise " << _name << ": Invalid option for format: "+format+" in "+name()+".  Valid options are 'rows' and 'columns'.");
    }
  }
  else if ((parameters.isParamValid("x")) ||
           (parameters.isParamValid("y")))
  {
    if (! ((parameters.isParamValid("x")) &&
           (parameters.isParamValid("y"))))
    {
      mooseError("In Piecewise " << _name << ": Both 'x' and 'y' must be specified if either one is specified.");
    }
    if (parameters.isParamValid("xy_data"))
    {
      mooseError("In Piecewise " << _name << ": Cannot specify 'x', 'y', and 'xy_data' together.");
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
      mooseError("In Piecewise " << _name << ": Length of data provided in 'xy_data' must be a multiple of 2.");
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
    mooseError("In Piecewise " << _name << ": Either 'data_file', 'x' and 'y', or 'xy_data' must be specified.");
  }

  try
  {
    _linear_interp = libmesh_make_unique<LinearInterpolation>(x, y);
  }
  catch (std::domain_error & e)
  {
    mooseError("In Piecewise " << _name << ": " << e.what());
  }

  if (parameters.isParamValid("axis"))
  {
    _axis=parameters.get<int>("axis");
    if (_axis < 0 || _axis > 2)
      mooseError("In Piecewise " << _name << ": axis="<<_axis<<" outside allowable range (0-2).");
    _has_axis = true;
  }
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
    while (size_t pos=line.find(','))
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
    mooseError("In Piecewise " << _name << ": Error opening file '" + _data_file_name + "'.");
  std::string line;

  unsigned int valid_line_index = 0;

  std::vector<Real> data;
  while (parseNextLineReals(file, data))
  {
    if (data.size() > 0)
    {
      if (valid_line_index == _x_index)
        x = data;
      else if (valid_line_index == _y_index)
        y = data;

      valid_line_index++;
    }
  }

  if (x.size() == 0)
    mooseError("In Piecewise " << _name << ": File '" + _data_file_name + "' with " << valid_line_index << " valid rows contains no x data.");

  if (y.size() == 0)
    mooseError("In Piecewise " << _name << ": File '" + _data_file_name + "' with " << valid_line_index << " valid rows contains no y data.");
  else if (y.size() != x.size())
    mooseError("In Piecewise " << _name << ": Lengths of x and y data do not match in file '" + _data_file_name + "'.");

  if (valid_line_index > 2  && _xy_only)
    mooseError("In Piecewise " << _name << ": Read more than two rows of data from file '" + _data_file_name + "'.  Did you mean to use \"format = columns\" or set \"xy_in_file_only\" to false?");
}

void
Piecewise::parseColumns( std::vector<Real> & x, std::vector<Real> & y )
{
  std::ifstream file(_data_file_name.c_str());
  if (!file.good())
    mooseError("In Piecewise " << _name << ": Error opening file '" + _data_file_name + "'.");

  std::vector<Real> scratch;
  unsigned int line_index = 0;
  while (parseNextLineReals(file, scratch))
  {
    if (scratch.size() > 0)
    {
      if (_x_index < scratch.size())
        x.push_back(scratch[_x_index]);
      else
        mooseError("In Piecewise " << _name << ": column " << _x_index << " for x does not exist on line " << line_index);

      if (_y_index < scratch.size())
        y.push_back(scratch[_y_index]);
      else
        mooseError("In Piecewise " << _name << ": column " << _y_index << " for y does not exist on line " << line_index);

      if (scratch.size() != 2 && _xy_only)
        mooseError("In Piecewise " << _name << ": Read more than 2 columns of data from file '" + _data_file_name + "'.  Did you mean to use \"format = rows\" or set \"xy_in_file_only\" to false?");
    }

    line_index++;
  }
}

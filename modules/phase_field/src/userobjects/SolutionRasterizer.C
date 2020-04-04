//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionRasterizer.h"

#include <fstream>

registerMooseObject("PhaseFieldApp", SolutionRasterizer);

InputParameters
SolutionRasterizer::validParams()
{
  InputParameters params = SolutionUserObject::validParams();
  params.addClassDescription("Process an XYZ file of atomic coordinates and filter atoms via "
                             "threshold or map variable values.");
  params.addRequiredParam<FileName>("xyz_input", "XYZ input file.");
  params.addRequiredParam<FileName>("xyz_output", "XYZ output file.");
  params.addRequiredParam<std::string>(
      "variable", "Variable from the mesh file to use for mapping to or filtering of the atoms.");
  MooseEnum modeEnum("MAP FILTER", "MAP");
  params.addParam<MooseEnum>("raster_mode", modeEnum, "Rasterization mode (MAP|FILTER).");
  params.addParam<Real>("threshold",
                        "Accept atoms with a variable value above this threshold in FILTER mode.");
  return params;
}

SolutionRasterizer::SolutionRasterizer(const InputParameters & parameters)
  : SolutionUserObject(parameters),
    _xyz_input(getParam<FileName>("xyz_input")),
    _xyz_output(getParam<FileName>("xyz_output")),
    _variable(getParam<std::string>("variable")),
    _raster_mode(getParam<MooseEnum>("raster_mode")),
    _threshold(0.0)
{
  if (_raster_mode == "FILTER")
  {
    if (!isParamValid("threshold"))
      mooseError("Please specify 'threshold' parameter for raster_mode = FILTER");
    _threshold = getParam<Real>("threshold");
  }
}

void
SolutionRasterizer::initialSetup()
{
  // only execute once
  if (_initialized)
    return;

  // initialize parent class
  SolutionUserObject::initialSetup();

  // open input XYZ file
  std::ifstream stream_in(_xyz_input.c_str());

  // open output XYZ file
  std::ofstream stream_out(_xyz_output.c_str());

  std::string line, dummy;
  Real x, y, z;
  unsigned int current_line = 0;
  unsigned int nfilter = 0, len0 = 0;
  while (std::getline(stream_in, line))
  {
    if (current_line < 2)
    {
      // dump header
      stream_out << line << '\n';

      // get length of line 0 - the amount of space we have to replace the atom count at the end of
      // filtering
      if (current_line == 0)
        len0 = line.size();
    }
    else
    {
      std::istringstream iss(line);

      if (iss >> dummy >> x >> y >> z)
        switch (_raster_mode)
        {
          case 0: // MAP
            stream_out << line << ' ' << pointValue(0.0, Point(x, y, z), _variable) << '\n';
            break;
          case 1: // FILTER
            if (pointValue(0.0, Point(x, y, z), _variable) > _threshold)
            {
              stream_out << line << '\n';
              nfilter++;
            }
            break;
        }
    }

    current_line++;
  }

  stream_in.close();
  stream_out.close();

  // modify output file to fix atom count in line 0
  if (_raster_mode == "FILTER")
  {
    // stringify the new number of atoms
    std::ostringstream oss;
    oss << nfilter;
    std::string newline0 = oss.str();

    // the new number should always be lower -> shorter than the old one, but we check to be sure
    if (newline0.size() > len0)
    {
      mooseWarning("SolutionRasterizer could not update XYZ atom count in header.");
      return;
    }

    // pad shorter numbers with spaces
    while (newline0.size() < len0)
      newline0 += ' ';

    // inject new number into the file
    std::ofstream stream_fix(_xyz_output.c_str(), std::ios::binary | std::ios::in | std::ios::out);
    stream_fix << newline0;
    stream_fix.close();
  }
}

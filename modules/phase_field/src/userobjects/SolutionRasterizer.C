#include "SolutionRasterizer.h"

template<>
InputParameters validParams<SolutionRasterizer>()
{
  InputParameters params = validParams<SolutionUserObject>();
  params.addClassDescription("Process an XYZ file of atomic coordinates and filter atoms via threshold or map variable values.");
  params.addRequiredParam<FileName>("xyz_input", "XYZ input file.");
  params.addRequiredParam<FileName>("xyz_output", "XYZ output file.");
  params.addRequiredParam<std::string>("variable", "Variable from the mesh file to use for mapping to or filtering of the atoms.");
  MooseEnum modeEnum("MAP FILTER", "MAP");
  params.addParam<MooseEnum>("raster_mode", modeEnum, "Rasterization mode (MAP|FILTER).");
  params.addParam<Real>("threshold", "Accept atoms with a variable value above this threshold in FILTER mode.");
  return params;
}

SolutionRasterizer::SolutionRasterizer(const std::string & name, InputParameters parameters) :
    SolutionUserObject(name, parameters),
    _xyz_input(getParam<FileName>("xyz_input")),
    _xyz_output(getParam<FileName>("xyz_output")),
    _variable(getParam<std::string>("variable")),
    _raster_mode(getParam<MooseEnum>("raster_mode")),
    _threshold(0.0)
{
  if (_raster_mode == "FILTER")
  {
    if (!isParamValid("threshold"))
      mooseError("Please specify 'threshold' parameter for raster_mode=FILTER");
    _threshold = getParam<Real>("threshold");
  }
}

void
SolutionRasterizer::initialSetup()
{
  // only execute once
  if (_initialized) return;

  // initialize parent class
  SolutionUserObject::initialSetup();

  // open input XYZ file
  std::ifstream stream_in(_xyz_input.c_str());

  // open output XYZ file
  std::ofstream stream_out(_xyz_output.c_str());

  std::string line, dummy;
  Real x, y, z;
  unsigned int current_line = 0;
  while (std::getline(stream_in, line))
  {
    if (current_line < 2)
    {
      // dump header (hmm... this will give the wrong number of atoms)
      stream_out << line << '\n';
    }
    else
    {
      std::istringstream iss(line);
      iss >> dummy >> x >> y >> z;

      if (iss.good())
        switch (_raster_mode)
        {
          case 0: // MAP
            stream_out << line << ' ' << pointValue(0.0, Point(x,y,z), _variable) << '\n';
            break;
          case 1: // FILTER
            if (pointValue(0.0, Point(x,y,z), _variable) > _threshold)
              stream_out << line << '\n';
            break;
        }
    }

    current_line++;
  }

  stream_in.close();
  stream_out.close();
}

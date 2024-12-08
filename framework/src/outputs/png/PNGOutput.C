//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseConfig.h"
#ifdef MOOSE_HAVE_LIBPNG

#include <fstream>
#include "PNGOutput.h"
#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "AuxiliarySystem.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", PNGOutput);

InputParameters
PNGOutput::validParams()
{
  InputParameters params = FileOutput::validParams();
  params.addParam<bool>("transparent_background",
                        false,
                        "Determination of whether the background will be transparent.");
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to use when creating the image");
  params.addParam<Real>("max", 1, "The maximum for the variable we want to use");
  params.addParam<Real>("min", 0, "The minimum for the variable we want to use");
  MooseEnum color("GRAY BRYW BWR RWB BR");
  params.addRequiredParam<MooseEnum>("color", color, "Choose the color scheme to use.");
  params.addRangeCheckedParam<unsigned int>(
      "resolution", 25, "resolution>0", "The length of the longest side of the image in pixels.");
  params.addRangeCheckedParam<Real>("out_bounds_shade",
                                    .5,
                                    "out_bounds_shade>=0 & out_bounds_shade<=1",
                                    "Color for the parts of the image that are out of bounds."
                                    "Value is between 1 and 0.");
  params.addRangeCheckedParam<Real>("transparency",
                                    1,
                                    "transparency>=0 & transparency<=1",
                                    "Value is between 0 and 1"
                                    "where 0 is completely transparent and 1 is completely opaque. "
                                    "Default transparency of the image is no transparency.");
  params.addClassDescription("Output data in the PNG format");
  return params;
}

PNGOutput::PNGOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _resolution(getParam<unsigned int>("resolution")),
    _color(parameters.get<MooseEnum>("color")),
    _transparent_background(getParam<bool>("transparent_background")),
    _transparency(getParam<Real>("transparency")),
    _nl_sys_num(libMesh::invalid_uint),
    _variable(getParam<VariableName>("variable")),
    _max(getParam<Real>("max")),
    _min(getParam<Real>("min")),
    _out_bounds_shade(getParam<Real>("out_bounds_shade"))
{
}

// Funtion for making the _mesh_function object.
void
PNGOutput::makeMeshFunc()
{

  // The number assigned to the variable.  Used to build the correct mesh.  Default is 0.
  unsigned int variable_number = 0;

  // PNGOutput does not currently scale for running in parallel.
  if (processor_id() != 0)
    mooseInfo("PNGOutput is not currently scalable.");

  bool var_found = false;
  if (_problem_ptr->getAuxiliarySystem().hasVariable(_variable))
  {
    variable_number = _problem_ptr->getAuxiliarySystem().getVariable(0, _variable).number();
    var_found = true;
  }

  else
    for (const auto nl_sys_num : make_range(_problem_ptr->numNonlinearSystems()))
      if (_problem_ptr->getNonlinearSystem(nl_sys_num).hasVariable(_variable))
      {
        variable_number =
            _problem_ptr->getNonlinearSystem(nl_sys_num).getVariable(0, _variable).number();
        _nl_sys_num = nl_sys_num;
        var_found = true;
      }

  if (!var_found)
    paramError("variable", "This doesn't exist.");

  const std::vector<unsigned int> var_nums = {variable_number};

  // If we want the background to be transparent, we need a number over 1.
  if (_transparent_background)
    _out_bounds_shade = 2;

  // Find the values that will be used for rescaling purposes.
  calculateRescalingValues();

  // Set up the mesh_function
  if (_nl_sys_num == libMesh::invalid_uint)
    _mesh_function = std::make_unique<libMesh::MeshFunction>(
        *_es_ptr,
        _problem_ptr->getAuxiliarySystem().serializedSolution(),
        _problem_ptr->getAuxiliarySystem().dofMap(),
        var_nums);
  else
    _mesh_function = std::make_unique<libMesh::MeshFunction>(
        *_es_ptr,
        _problem_ptr->getNonlinearSystem(_nl_sys_num).serializedSolution(),
        _problem_ptr->getNonlinearSystem(_nl_sys_num).dofMap(),
        var_nums);
  _mesh_function->init();

  // Need to enable out of mesh with the given control color scaled in reverse
  // so when scaling is done, this value retains it's original value.
  _mesh_function->enable_out_of_mesh_mode(reverseScale(_out_bounds_shade));
}

// Function to find the min and max values so that all the values can be scaled between the two.
void
PNGOutput::calculateRescalingValues()
{
  // The max and min.
  // If the max value wasn't specified in the input file, find it from the system.
  if (!_pars.isParamSetByUser("max"))
  {
    if (_nl_sys_num == libMesh::invalid_uint)
      _scaling_max = _problem_ptr->getAuxiliarySystem().serializedSolution().max();
    else
      _scaling_max = _problem_ptr->getNonlinearSystem(_nl_sys_num).serializedSolution().max();
  }
  else
    _scaling_max = _max;

  // If the min value wasn't specified in the input file, find it from the system.
  if (!_pars.isParamSetByUser("min"))
  {
    if (_nl_sys_num == libMesh::invalid_uint)
      _scaling_min = _problem_ptr->getAuxiliarySystem().serializedSolution().min();
    else
      _scaling_min = _problem_ptr->getNonlinearSystem(_nl_sys_num).serializedSolution().min();
  }
  else
    _scaling_min = _min;

  // The amount the values will need to be shifted.
  _shift_value = 0;

  // Get the shift value.
  if (_scaling_min != 0)
  {
    // Shiftvalue will be the same magnitude, but
    // going in the opposite direction of the scalingMin
    _shift_value -= _scaling_min;
  }

  // Shift the max.
  _scaling_max += _shift_value;
}

// Function to apply the scale to the data points.
// Needed to be able to see accurate images that cover the appropriate color spectrum.
inline Real
PNGOutput::applyScale(Real value_to_scale)
{
  return ((value_to_scale + _shift_value) / _scaling_max);
}

// Function to reverse the scaling that happens to a value.
// Needed to be able to accurately control the _out_bounds_shade.
inline Real
PNGOutput::reverseScale(Real value_to_unscale)
{
  return ((value_to_unscale * _scaling_max) - _shift_value);
}

// Function that controls the colorization of the png image for non-grayscale images.
void
PNGOutput::setRGB(png_byte * rgb, Real selection)
{
  // With this system we have a color we start with when the value is 0 and another it approaches as
  // the value increases all the way to 255.  If we want it to approach another color from that new
  // color, it will do so for the next 255, so the transition is from 256 - 511.  For each
  // additional color we want to transition to, we need another 255. Transitioning from no color, or
  // black to Red then Green then Blue then the values of from black as it becomes Red would be 0 -
  // 255, Red to Green as 256 - 511 and then Green to Blue as 512 - 767 which gives us our total
  // colorSpectrum of 0 - 767, which includes those colors and each of their states in the
  // transistion.
  unsigned int number_of_destination_colors = 1;
  switch (_color)
  {
    // BRYW.  Three destination colors (R,Y,W).
    case 1:
      number_of_destination_colors = 3;
      break;

    // BWR.  Two destination colors (W,R).
    case 2:
      number_of_destination_colors = 2;
      break;

    // RWB.  Two destination colors (W,B).
    case 3:
      number_of_destination_colors = 2;
      break;

    // BR.   One destination color (R).
    case 4:
      number_of_destination_colors = 1;
      break;
  }

  // We need to convert the number of colors into the spectrum max, then convert the value from the
  // mesh to a point somewhere in the range of 0 to color_spectrum_max.
  auto color_spectrum_max = (256 * number_of_destination_colors) - 1;
  auto color = (unsigned int)(selection * color_spectrum_max);

  // Unless we specifically say some part is transparent, we want the whole image to be opaque.
  auto tran = (unsigned int)(_transparency * 255);

  // Make sure everything is within our colorSpectrum.  If it's bigger, then we want a
  // transparent background.
  if (color > color_spectrum_max)
  {
    color = color_spectrum_max;
    tran = 0;
  }

  auto magnitude = color % 256;

  switch (_color)
  {
    // Current color scheme: Blue->Red->Yellow->White
    case 1:
      // Blue->Red
      if (color < 256)
      {
        rgb[0] = magnitude;
        rgb[1] = 0;
        rgb[2] = 50; // 255 - magnitude;
      }
      // Red->Yellow
      else if (color < 512)
      {
        rgb[0] = 255;
        rgb[1] = magnitude;
        rgb[2] = 0;
      }
      // Yellow->White
      else
      {
        rgb[0] = 255;
        rgb[1] = 255;
        rgb[2] = magnitude;
      }
      break;

    // Color Scheme: Blue->White->Red
    // Using the RGB values found in Paraview
    case 2:
      // Blue->White
      if (color < 256)
      {
        rgb[0] = (int)(255.0 * (0.231373 + (0.002485 * (float)magnitude)));
        rgb[1] = (int)(255.0 * (0.298039 + (0.002223 * (float)magnitude)));
        rgb[2] = (int)(255.0 * (0.752941 + (0.000439 * (float)magnitude)));
      }
      // White->Red
      else
      {
        rgb[0] = (int)(255.0 * (0.865003 - (0.000624 * (float)magnitude)));
        rgb[1] = (int)(255.0 * (0.865003 - (0.003331 * (float)magnitude)));
        rgb[2] = (int)(255.0 * (0.865003 - (0.002808 * (float)magnitude)));
      }
      break;

    // Red->White->Blue
    case 3:
      // Red->White
      if (color < 256)
      {
        rgb[0] = 255;
        rgb[1] = magnitude;
        rgb[2] = magnitude;
      }
      // White->Blue
      else
      {
        rgb[0] = 255 - magnitude;
        rgb[1] = 255 - magnitude;
        rgb[2] = 255;
      }
      break;

    // Blue->Red
    case 4:
      // Blue->Red
      rgb[0] = magnitude;
      rgb[1] = 0;
      rgb[2] = 255 - magnitude;
      break;
  }
  // Add any transparency.
  rgb[3] = tran;
}

void
PNGOutput::output()
{
  makeMeshFunc();
  _box = MeshTools::create_bounding_box(*_mesh_ptr);

  // Make sure this happens on processor 0
  if (processor_id() == 0)
    makePNG();
}

// Function the writes the PNG out to the appropriate filename.
void
PNGOutput::makePNG()
{
  // Get the max and min of the BoundingBox
  Point max_point = _box.max();
  Point min_point = _box.min();

  // The the total distance on the x and y axes.
  Real dist_x = max_point(0) - min_point(0);
  Real dist_y = max_point(1) - min_point(1);

  // Width and height for the PNG image.
  Real width;
  Real height;

  // Variable to record the resolution variable after normalized to work with pixels in longest
  // direction.
  Real normalized_resolution;

  // The longer dimension becomes the value to which we scale the other.
  if (dist_x > dist_y)
  {
    width = _resolution;
    height = (_resolution / dist_x) * dist_y;
    normalized_resolution = (((Real)_resolution) / dist_x);
  }
  else
  {
    height = _resolution;
    width = (_resolution / dist_y) * dist_x;
    normalized_resolution = (((Real)_resolution) / dist_y);
  }

  // Create the filename based on base and the test step number.
  std::ostringstream png_file;
  png_file << _file_base << "_" << std::setfill('0') << std::setw(3) << _t_step << ".png";

  // libpng is built on C, so by default it takes FILE*.
  FILE * fp = nullptr;
  png_structp pngp = nullptr;
  png_infop infop = nullptr;
  // Required depth for proper image clarity.
  Real depth = 8;
  // Allocate resources.
  std::vector<png_byte> row((width + 1) * 4);

  // Check if we can open and write to the file.
  MooseUtils::checkFileWriteable(png_file.str());

  // Open the file with write and bit modes.
  fp = fopen(png_file.str().c_str(), "wb");

  pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!pngp)
    mooseError("Failed to make the pointer string for the png.");

  infop = png_create_info_struct(pngp);
  if (!infop)
    mooseError("Failed to make an info pointer for the png.");

  // Initializes the IO for the png.  Needs FILE* to compile.
  png_init_io(pngp, fp);

  // Set up the PNG header.
  png_set_IHDR(pngp,
               infop,
               width,
               height,
               depth,
               (_color ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_GRAY),
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(pngp, infop);

  // Initiallizing the point that will be used for populating the mesh values.
  // Initializing x, y, z to zero so that we don't access the point before it's
  // been set.  z = 0 for all the png's.
  Point pt(0, 0, 0);

  // Dense vector that we can pass into the _mesh_function to fill with a value for a given point.
  DenseVector<Number> dv(0);

  // Loop through to create the image.
  for (Real y = max_point(1); y >= min_point(1); y -= 1. / normalized_resolution)
  {
    pt(1) = y;
    unsigned int index = 0;
    for (Real x = min_point(0); x <= max_point(0); x += 1. / normalized_resolution)
    {
      pt(0) = x;
      (*_mesh_function)(pt, _time, dv, nullptr);

      // Determine whether to create the PNG in color or grayscale
      if (_color)
        setRGB(&row.data()[index * 4], applyScale(dv(0)));
      else
        row.data()[index] = applyScale(dv(0)) * 255;

      index++;
    }
    png_write_row(pngp, row.data());
  }

  // Close the file and take care of some other png end stuff.
  png_write_end(pngp, nullptr);
  if (fp != nullptr)
    fclose(fp);
  if (infop != nullptr)
    png_free_data(pngp, infop, PNG_FREE_ALL, -1);
  if (pngp != nullptr)
    png_destroy_write_struct(&pngp, &infop);
}

#endif

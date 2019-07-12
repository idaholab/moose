//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_HAVE_LIBPNG

#include <fstream>
#include "PNGOutput.h"
#include "FEProblemBase.h"
#include "NonlinearSystem.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", PNGOutput);

template <>
InputParameters
validParams<PNGOutput>()
{
  InputParameters params = validParams<FileOutput>();
  params.addParam<bool>("transparent_background",
                        false,
                        "Determination of whether the background will be transparent.");
  params.addParam<unsigned int>("resolution", 1, "The resolution of the image.");
  MooseEnum color("GRAY BRYW BCR RWB BR");
  params.addParam<MooseEnum>("color", color, "Choose the color scheme to use.");
  params.addRangeCheckedParam<Real>("out_bounds_shade",
                                    .5,
                                    "out_bounds_shade>=0 & out_bounds_shade<=1",
                                    "Color for the parts of the image that are out of bounds."
                                    "Value is between 1 and 0.");
  params.addRangeCheckedParam<Real>("transparency",
                                    1,
                                    "transparency>=0 & transparency<=1",
                                    "Value is between 1 and 0"
                                    "where 1 is completely opaque and 0 is completely transparent"
                                    "Default transparency of the image is no transparency.");

  return params;
}

PNGOutput::PNGOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _resolution(getParam<unsigned int>("resolution")),
    _color(parameters.get<MooseEnum>("color")),
    _transparent_background(getParam<bool>("transparent_background")),
    _transparency(getParam<Real>("transparency")),
    _out_bounds_shade(getParam<Real>("out_bounds_shade"))
{
}

// Funtion for making the _mesh_function object.
void
PNGOutput::makeMeshFunc()
{

  const std::vector<unsigned int> var_nums = {0};

  // If we want the background to be transparent, we need a number over 1.
  if (_transparent_background)
    _out_bounds_shade = 2;
  // Find the values that will be used for rescaling purposes.
  calculateRescalingValues();

  // Set up the mesh_function
  _mesh_function =
      libmesh_make_unique<MeshFunction>(*_es_ptr,
                                        _problem_ptr->getNonlinearSystem().serializedSolution(),
                                        _problem_ptr->getNonlinearSystem().dofMap(),
                                        var_nums);
  _mesh_function->init();

  // Need to enable out of mesh with the given control color scaled in reverse
  // scaling is done, this value retains it's original value.
  _mesh_function->enable_out_of_mesh_mode(reverseScale(_out_bounds_shade));
}

// Function to find the min and max values so that all the values can be scaled between the two.
void
PNGOutput::calculateRescalingValues()
{
  // The min and max.
  _scaling_min = _problem_ptr->getNonlinearSystem().serializedSolution().min();
  _scaling_max = _problem_ptr->getNonlinearSystem().serializedSolution().max();
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
  // Using an RGB system with Red as 0 - 255, Green as 256 - 511 and
  // Blue as 512 - 767 gives us our total colorSpectrum of 0 - 767.
  // Depending on the color scheme we're using, we may want to use a subset
  // of the colorSpectrum.
  auto color_spectrum_max = 767;
  switch (_color)
  {
    // BRYW.  Keep the spectum as is.
    case 1:
      break;
    // BCR.  Change spectrum.
    case 2:
    // RWB.  Change spectrum.
    case 3:
      color_spectrum_max = 1013; // 511;
      break;
    case 4:
      color_spectrum_max = 255;
      break;
  }

  // We need to convert the
  auto color = (int)(selection * color_spectrum_max);
  auto tran = (int)(_transparency * 255);
  // Make sure everything is within our colorSpectrum.

  if (color > color_spectrum_max)
  {
    color = color_spectrum_max;
    tran = 0;
  }
  if (color < 0)
    color = 0;
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

    // Color Scheme: Blue->Cream->Red
    case 2:
      // Blue->Cream
      if (color < 256)
      {
        rgb[0] = magnitude;
        rgb[1] = magnitude - (int)(5.0 / (256.0 - (float)magnitude));
        rgb[2] = 255 - (int)(40.0 / (256.0 - (float)magnitude));
      }
      // Cream->Red
      else
      {
        rgb[0] = 255;
        rgb[1] = 255 - (5 / (magnitude + 1)) - (magnitude + (5 / (magnitude + 1)));
        rgb[2] = 255 - (40 / (magnitude + 1)) - (magnitude + (40 / (magnitude + 1)));
      }
      break;

    // Inverted form.
    // Red->White->Blue
    case 3:
      // Red->White
      if (color < 256)
      {
        rgb[0] = magnitude + (int)(100 / (magnitude + 1));
        rgb[1] = 0;
        rgb[2] = 0;
      }
      else if (color < 512)
      {
        rgb[0] = 255;
        rgb[1] = magnitude - (int)(100.0 * ((float)magnitude / 255.0));
        rgb[2] = magnitude - (int)(100.0 * ((float)magnitude / 255.0));
      }
      // White->Blue
      else if (color < 768)
      {
        rgb[0] = 255 - (int)(100.0 * (1.0 + ((float)magnitude) / (100.0 * (255.0 / 155.0))));
        rgb[1] = 255 - (int)(100.0 * (1.0 + ((float)magnitude) / (100.0 * (255.0 / 155.0))));
        rgb[2] = 255;
      }
      else
      {
        rgb[0] = 0;
        rgb[1] = 0;
        rgb[2] = 255 - (int)(magnitude / (255 / 100));
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
PNGOutput::output(const ExecFlagType & /*type*/)
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
  Point maxPoint = _box.max();
  Point minPoint = _box.min();

  // The the total distance on the x and y axes.
  Real dist_x = maxPoint(0) - minPoint(0);
  Real dist_y = maxPoint(1) - minPoint(1);

  // Create the filename based on base and the test step number.
  std::ostringstream png_file;
  png_file << _file_base << "_" << std::setfill('0') << std::setw(3) << _t_step << ".png";

  // libpng is built on C, so by default it takes FILE*.
  FILE * fp = nullptr;
  png_structp pngp = nullptr;
  png_infop infop = nullptr;
  // Required depth for proper image clarity.
  Real depth = 8;
  Real width = dist_x * _resolution;
  Real height = dist_y * _resolution;
  // Allocate resources.
  std::vector<png_byte> row((width * 4) + 1);

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
  for (Real y = maxPoint(1); y >= minPoint(1); y -= 1. / _resolution)
  {
    pt(1) = y;
    int indx = 0;
    for (Real x = minPoint(0); x <= maxPoint(0); x += 1. / _resolution)
    {
      pt(0) = x;
      (*_mesh_function)(pt, _time, dv, nullptr);

      // Determine whether to create the PNG in color or grayscale
      if (_color)
        setRGB(&row.data()[indx * 4], applyScale(dv(0)));
      else
        row.data()[indx] = applyScale(dv(0)) * 255;

      indx++;
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
    png_destroy_write_struct(&pngp, (png_infopp) nullptr);
}

#endif

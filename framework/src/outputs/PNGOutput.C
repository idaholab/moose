//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  params.addParam<unsigned int>("resolution", 2000, "The resolution of the image.");
  params.addParam<bool>("color", false, "Show the image in color?");
  params.addRangeCheckedParam<Real>("_out_bounds_shade",
                                    .5,
                                    "_out_bounds_shade>=0 & _out_bounds_shade<=1",
                                    "Color for the parts of the image that are out of bounds."
                                    "Value is between 1 and 0.");
  return params;
}

PNGOutput::PNGOutput(const InputParameters & parameters)
  : FileOutput(parameters),
    _resolution(getParam<unsigned int>("resolution")),
    _color(getParam<bool>("color")),
    _out_bounds_shade(getParam<Real>("_out_bounds_shade"))
{
}

// Funtion for making the _mesh_function object.
void
PNGOutput::makeMeshFunc()
{

  const std::vector<unsigned int> var_nums = {0};

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

inline Real
PNGOutput::applyScale(Real value_to_scale)
{
  return ((value_to_scale + _shift_value) / _scaling_max);
}

inline Real
PNGOutput::reverseScale(Real value_to_unscale)
{
  return ((value_to_unscale * _scaling_max) - _shift_value);
}

void
PNGOutput::setRGB(png_byte * rgb, Real selection)
{
  // Using an RGB system with Red as 0 - 255, Green as 256 - 511 and
  // Blue as 512 - 767 gives us our total colorSpectrum of 0 - 767.
  const auto color_spectrum_max = 767;
  // We need to convert the
  auto color = (int)(selection * color_spectrum_max);
  // Make sure everything is within our colorSpectrum.
  if (color > color_spectrum_max)
    color = color_spectrum_max;
  if (color < 0)
    color = 0;
  auto magnitude = color % 256;

  // Current color scheme: Blue->Red->Yellow->White
  // Blue->Red
  if (color < 256)
  {
    rgb[0] = magnitude;
    rgb[1] = 0;
    rgb[2] = 255 - magnitude;
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

  // // Color Scheme: Blue->Cream->Red
  // // Blue->Cream
  // if (color < 256)
  // {
  //   rgb[0] = magnitude - (40/(256-magnitude));
  //   rgb[1] = magnitude - (5/(256-magnitude));
  //   rgb[2] = 255 - (40/(256-magnitude));
  // }
  // // Cream->Red
  // else
  // {
  //   rgb[0] = 255 - (40/(256-magnitude));
  //   rgb[1] = (255 - (5/(magnitude+1))) - (magnitude + (5/(magnitude+1)));
  //   rgb[2] = (255 - (40/(magnitude+1))) - (magnitude + (40/(magnitude+1)));
  // }
}

void
PNGOutput::output(const ExecFlagType & /*type*/)
{
  makeMeshFunc();
  _box = MeshTools::create_bounding_box(*_mesh_ptr);
  if (processor_id() == 0)
    makePNG();
}

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

  FILE * fp = nullptr;
  png_structrp pngp = nullptr;
  png_infop infop = nullptr;
  // Required depth for image clarity.
  Real depth = 8;
  Real width = dist_x * _resolution;
  Real height = dist_y * _resolution;
  // Allocate resources.
  std::vector<png_byte> row ((width * 3) + 1);

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

  png_init_io(pngp, fp);

  // Set up the PNG header.
  png_set_IHDR(pngp,
               infop,
               width,
               height,
               depth,
               (_color ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY),
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(pngp, infop);


  // Initiallizing the point that will be used for populating the mesh values.
  // Initializing x, y, z to zero so that we don't access the point before it's
  // been set.  z = 0 for all the png's.
  Point pt(0, 0, 0);

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
        setRGB(&row.data()[indx * 3], applyScale(dv(0)));
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

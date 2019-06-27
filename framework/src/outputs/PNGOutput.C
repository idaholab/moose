//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#define nls _problem_ptr->getNonlinearSystem()
#define scaledValue ((dv(0) + shiftValue) / scalingMax)
#define reverseScale ((outBoundsControlColor * scalingMax) - shiftValue)

#include <fstream>
#include "PNGOutput.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", PNGOutput);

template <>
InputParameters
validParams<PNGOutput>()
{
  InputParameters params = validParams<Output>();
  params.addParam<Real>("resolution", 2000, "The resolution of the image.");
  params.addParam<std::string>("PNGFile", "Adam", "Root filename of the PNG to be created.");
  params.addParam<Real>("testStepToPNG", -1, "PNG to save.");
  params.addParam<bool>("inColor", false, "Show the image in color?");
  params.addParam<Real>(
      "outBoundsControlColor", .5, "Color for the parts of the image that are out of bounds");
  return params;
}

PNGOutput::PNGOutput(const InputParameters & parameters)
  : Output(parameters),
    resolution(getParam<Real>("resolution")),
    PNGFile(getParam<std::string>("PNGFile")),
    testStepToPNG(getParam<Real>("testStepToPNG")),
    inColor(getParam<bool>("inColor")),
    outBoundsControlColor(getParam<Real>("outBoundsControlColor"))
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
      libmesh_make_unique<MeshFunction>(*_es_ptr, nls.serializedSolution(), nls.dofMap(), var_nums);
  _mesh_function->init();

  // Need to enable out of mesh with the given control color scaled in reverse
  // scaling is done, this value retains it's original value.
  _mesh_function->enable_out_of_mesh_mode(reverseScale);
}

void
PNGOutput::calculateRescalingValues()
{
  // The min and max.
  scalingMin = nls.serializedSolution().min();
  scalingMax = nls.serializedSolution().max();
  shiftValue = 0;

  // Get the shift value.
  if (scalingMin != 0)
  {
    // Shiftvalue will be the same magnitude, but
    // going in the opposite direction of the scalingMin
    shiftValue -= scalingMin;
  }

  // Shift the max.
  scalingMax += shiftValue;
}

void
PNGOutput::setRGB(png_byte * rgb, Real selection)
{
  // Using an RGB system with Red as 0 - 255, Green as 256 - 511 and
  // Blue as 512 - 767 gives us our total colorSpectrum of 0 - 767.
  const Real colorSpectrumMax = 767;
  // We need to convert the
  auto color = (int)(selection * colorSpectrumMax);
  // Make sure everything is within our colorSpectrum.
  if (color > colorSpectrumMax)
    color = colorSpectrumMax;
  if (color < 0)
    color = 0;
  Real magnitude = color % 256;

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
}

void
PNGOutput::output(const ExecFlagType & /*type*/)
{
  makeMeshFunc();
  box = MeshTools::create_bounding_box(*_mesh_ptr);
  makePNG();
}

void
PNGOutput::makePNG()
{
  // Increment the testStep
  testStep++;

  // Get the max and min of the BoundingBox
  Point maxPoint = box.max();
  Point minPoint = box.min();

  // The the total distance on the x and y axes.
  Real distx = maxPoint(0) - minPoint(0);
  Real disty = maxPoint(1) - minPoint(1);

  // Create the filename based on base and the test step number.
  std::string PNGFile2 = PNGFile;
  PNGFile2 += std::to_string((int)testStep);
  PNGFile2 += ".png";

  FILE * fp = nullptr;
  png_structrp pngp = nullptr;
  png_infop infop = nullptr;
  png_bytep row = nullptr;
  Real depth = 8;
  Real width = distx * resolution;
  Real height = disty * resolution;

  if (testStepToPNG != -1)
    if (testStep != testStepToPNG)
      return;

  fp = fopen(PNGFile2.c_str(), "wb");

  if (!fp)
  {
    mooseError("Failed to open the file for the PNG output.");
    return;
  }

  pngp = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  if (!pngp)
  {
    mooseError("Failed to make the pointer string for the png.");
    return;
  }

  infop = png_create_info_struct(pngp);
  if (!infop)
  {
    mooseError("Failed to make an info pointer for the png.");
    return;
  }

  png_init_io(pngp, fp);

  // Set up the PNG header.
  png_set_IHDR(pngp,
               infop,
               width,
               height,
               depth,
               (inColor ? PNG_COLOR_TYPE_RGB : PNG_COLOR_TYPE_GRAY),
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(pngp, infop);

  // Allocate resources.
  row = new png_byte[(width * 3) + 1];

  Point pt(1, 1, 0);

  DenseVector<Number> dv(0);

  // Loop through to create the image.
  for (Real y = maxPoint(1); y >= minPoint(1); y -= 1 / resolution)
  {
    int indx = 0;
    for (Real x = minPoint(0); x <= maxPoint(0); x += 1 / resolution)
    {
      pt(0) = x;
      (*_mesh_function)(pt, _time, dv, nullptr);

      // Determine whether to create the PNG in color or grayscale
      if (inColor)
        setRGB(&row[indx * 3], scaledValue);
      else
        row[indx] = scaledValue * 255;

      indx++;
    }

    pt(1) = y;
    png_write_row(pngp, row);
  }

  png_write_end(pngp, nullptr);
  if (fp != nullptr)
    fclose(fp);
  if (infop != nullptr)
    png_free_data(pngp, infop, PNG_FREE_ALL, -1);
  if (pngp != nullptr)
    png_destroy_write_struct(&pngp, (png_infopp) nullptr);
  if (row != nullptr)
    delete[] row;
}

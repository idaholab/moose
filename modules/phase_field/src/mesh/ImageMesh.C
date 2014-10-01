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

#include <cstdlib> // std::system, mkstemp
#include "ImageMesh.h"
#include "pcrecpp.h"

// libMesh includes
#include "libmesh/mesh_generation.h"

template<>
InputParameters validParams<ImageMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();
  params.addRequiredParam<FileName>("image_file", "Name of the image file to extract mesh parameters from");
  params.addParam<bool>("scale_to_one", true, "Whether or not to scale the image so its max dimension is 1");
  params.addRangeCheckedParam<Real>("cells_per_pixel", 1.0, "cells_per_pixel<=1.0", "The number of mesh cells per pixel, must be <=1 ");
  return params;
}

ImageMesh::ImageMesh(const std::string & name, InputParameters parameters) :
    GeneratedMesh(name, parameters),
    _image_file(getParam<FileName>("image_file")),
    _scale_to_one(getParam<bool>("scale_to_one")),
    _cells_per_pixel(getParam<Real>("cells_per_pixel"))
{
}

ImageMesh::ImageMesh(const ImageMesh & other_mesh) :
    GeneratedMesh(other_mesh)
{
}

ImageMesh::~ImageMesh()
{
}

MooseMesh &
ImageMesh::clone() const
{
  return *(new ImageMesh(*this));
}

void
ImageMesh::buildMesh()
{
  // For reporting possible error messages
  std::string error_message = "";

  // A template for creating a temporary file.
  char temp_file[] = "file_command_output.XXXXXX";

  // Use a do-loop so we can break out under various error conditions
  // while still cleaning up temporary files.  Basically use goto
  // statements without actually using them.
  do
  {
    // mkstemp is not in namespace std for whatever reason...
    int fd = mkstemp(temp_file);

    // If mkstemp fails, we failed.
    if (fd == -1)
    {
      error_message = "Error creating temporary file in ImageMesh::buildMesh()";
      break;
    }

    // Construct the command string
    std::ostringstream command;
    command << "file " << _image_file << " 2>/dev/null 1>" << temp_file;

    // Make the system call, catch the return code
    int exit_status = std::system(command.str().c_str());

    // If the system command returned a non-zero status, we failed.
    if (exit_status != 0)
    {
      error_message = "Error calling 'file' command in ImageMesh::buildMesh()";
      break;
    }

    // Open the file which contains the result of the system command
    std::ifstream fin(temp_file);

    // Read the contents of the output file into a string
    std::string command_result;
    std::getline(fin, command_result);

    // A regular expression which matches "NNN x NNN" , i.e. any number
    // of digits, a space, an 'x', a space, and any number of digits.
    // The parentheses define capture groups which are stored into the
    // xsize and ysize integers.
    // Here's an example string:
    // sixteenth_image001_cropped3_closing_298.png: PNG image data, 115 x 99, 16-bit/color RGB, non-interlaced
    int xpixels = 0, ypixels = 0;
    pcrecpp::RE re("(\\d+) x (\\d+)");
    re.PartialMatch(command_result, &xpixels, &ypixels);

    // Detect failure of the regex
    if ((xpixels==0) || (ypixels==0))
    {
      error_message = "Regex failed to find a match in " + command_result;
      break;
    }

    // Set the maximum dimension to 1.0 while scaling the other
    // direction to maintain the aspect ratio.
    Real
      xmax = xpixels,
      ymax = ypixels;

    if (_scale_to_one)
    {
      Real max = std::max(xmax, ymax);
      xmax /= max;
      ymax /= max;
    }

    // Compute the number of cells in the x and y direction based on
    // the user's cells_per_pixel parameter.  Note: we use ints here
    // because the GeneratedMesh params object uses ints for these...
    int
      nx = static_cast<int>(_cells_per_pixel * xpixels),
      ny = static_cast<int>(_cells_per_pixel * ypixels);

    // Actually build the Mesh
    MeshTools::Generation::build_square(dynamic_cast<UnstructuredMesh&>(getMesh()),
                                        nx, ny,
                                        /*xmin=*/0., /*xmax=*/xmax,
                                        /*ymin=*/0., /*ymax=*/ymax,
                                        QUAD4);

    // We've determined the correct xmax, ymax values so set them in
    // our InputParameters object
    _pars.set<Real>("xmax") = xmax;
    _pars.set<Real>("ymax") = ymax;

    // Set the number of cells in the x and y directions that we determined.
    _pars.set<int>("nx") = nx;
    _pars.set<int>("ny") = ny;

  } while (false);

  // Remove the temporary file.  This will still work even if the file was never created...
  std::remove(temp_file);

  // Report and exit if there was an error
  if (error_message != "")
    mooseError(error_message);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImageMesh.h"
#include "pcrecpp.h"
#include "MooseApp.h"
#include "MooseTypes.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/face_quad4.h"

#include <cstdlib> // std::system, mkstemp
#include <fstream>

#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", ImageMesh);

InputParameters
ImageMesh::validParams()
{
  InputParameters params = GeneratedMesh::validParams();
  params += FileRangeBuilder::validParams();
  params.addClassDescription("Generated mesh with the aspect ratio of a given image stack.");

  // Add ImageMesh-specific params
  params.addParam<bool>(
      "scale_to_one", true, "Whether or not to scale the image so its max dimension is 1");
  params.addRangeCheckedParam<Real>("cells_per_pixel",
                                    1.0,
                                    "cells_per_pixel<=1.0",
                                    "The number of mesh cells per pixel, must be <=1 ");

  return params;
}

ImageMesh::ImageMesh(const InputParameters & parameters)
  : GeneratedMesh(parameters),
    FileRangeBuilder(parameters),
    _scale_to_one(getParam<bool>("scale_to_one")),
    _cells_per_pixel(getParam<Real>("cells_per_pixel"))
{
  // Failure is not an option
  errorCheck();
}

ImageMesh::ImageMesh(const ImageMesh & other_mesh)
  : GeneratedMesh(other_mesh),
    FileRangeBuilder(other_mesh.parameters()),
    _scale_to_one(getParam<bool>("scale_to_one")),
    _cells_per_pixel(getParam<Real>("cells_per_pixel"))
{
}

std::unique_ptr<MooseMesh>
ImageMesh::safeClone() const
{
  return _app.getFactory().copyConstruct(*this);
}

void
ImageMesh::buildMesh()
{
  // A list of filenames of length 1 means we are building a 2D mesh
  if (_filenames.size() == 1)
    buildMesh2D(_filenames[0]);

  else
    buildMesh3D(_filenames);
}

void
ImageMesh::buildMesh3D(const std::vector<std::string> & filenames)
{
  // If the user gave us a "stack" with 0 or 1 files in it, we can't
  // really create a 3D Mesh from that
  if (filenames.size() <= 1)
    mooseError("ImageMesh error: Cannot create a 3D ImageMesh from an image stack with ",
               filenames.size(),
               " images.");

  // For each file in the stack, process it using the 'file' command.
  // We want to be sure that all the images in the stack are the same
  // size, for example...
  int xpixels = 0, ypixels = 0, zpixels = filenames.size();

  // Take pixel info from the first image in the stack to determine the aspect ratio
  GetPixelInfo(filenames[0], xpixels, ypixels);

  // TODO: Check that all images are the same aspect ratio and have
  // the same number of pixels?  ImageFunction does not currently do
  // this...
  // for (const auto & filename : filenames)
  // {
  //   // Extract the number of pixels from the image using the file command
  //   GetPixelInfo(filename, xpixels, ypixels);
  //
  //   // Moose::out << "Image " << filename << " has size: " << xpixels << " by " << ypixels <<
  //   std::endl;
  // }

  // Use the number of x and y pixels and the number of images to
  // determine the the x, y, and z dimensions of the mesh.  We assume
  // that there is 1 pixel in the z-direction for each image in the
  // stack.

  // Set the maximum dimension to 1.0 while scaling the other
  // directions to maintain the aspect ratio.
  _xmax = xpixels;
  _ymax = ypixels;
  _zmax = zpixels;

  if (_scale_to_one)
  {
    Real max = std::max(std::max(_xmax, _ymax), _zmax);
    _xmax /= max;
    _ymax /= max;
    _zmax /= max;
  }

  // Compute the number of cells in the x and y direction based on
  // the user's cells_per_pixel parameter.  Note: we use ints here
  // because the GeneratedMesh params object uses ints for these...
  _nx = static_cast<int>(_cells_per_pixel * xpixels);
  _ny = static_cast<int>(_cells_per_pixel * ypixels);
  _nz = static_cast<int>(_cells_per_pixel * zpixels);

  // Actually build the Mesh
  MeshTools::Generation::build_cube(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                    _nx,
                                    _ny,
                                    _nz,
                                    /*xmin=*/0.,
                                    /*xmax=*/_xmax,
                                    /*ymin=*/0.,
                                    /*ymax=*/_ymax,
                                    /*zmin=*/0.,
                                    /*zmax=*/_zmax,
                                    HEX8);
}

void
ImageMesh::buildMesh2D(const std::string & filename)
{
  int xpixels = 0, ypixels = 0;

  // Extract the number of pixels from the image using the file command
  GetPixelInfo(filename, xpixels, ypixels);

  // Set the maximum dimension to 1.0 while scaling the other
  // direction to maintain the aspect ratio.
  _xmax = xpixels;
  _ymax = ypixels;

  if (_scale_to_one)
  {
    Real max = std::max(_xmax, _ymax);
    _xmax /= max;
    _ymax /= max;
  }

  // Compute the number of cells in the x and y direction based on
  // the user's cells_per_pixel parameter.  Note: we use ints here
  // because the GeneratedMesh params object uses ints for these...
  _nx = static_cast<int>(_cells_per_pixel * xpixels);
  _ny = static_cast<int>(_cells_per_pixel * ypixels);

  // Actually build the Mesh
  MeshTools::Generation::build_square(dynamic_cast<UnstructuredMesh &>(getMesh()),
                                      _nx,
                                      _ny,
                                      /*xmin=*/0.,
                                      /*xmax=*/_xmax,
                                      /*ymin=*/0.,
                                      /*ymax=*/_ymax,
                                      QUAD4);
}

void
ImageMesh::GetPixelInfo(std::string filename, int & xpixels, int & ypixels)
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
    command << "file " << filename << " 2>/dev/null 1>" << temp_file;

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
    // sixteenth_image001_cropped3_closing_298.png: PNG image data, 115 x 99, 16-bit/color RGB,
    // non-interlaced
    xpixels = 0, ypixels = 0;
    pcrecpp::RE re("(\\d+) x (\\d+)");
    re.PartialMatch(command_result, &xpixels, &ypixels);

    // Detect failure of the regex
    if ((xpixels == 0) || (ypixels == 0))
    {
      error_message = "Regex failed to find a match in " + command_result;
      break;
    }
  } while (false);

  // Remove the temporary file.  This will still work even if the file was never created...
  std::remove(temp_file);

  // Report and exit if there was an error
  if (error_message != "")
    mooseError(error_message);
}

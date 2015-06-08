/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include <cstdlib> // std::system, mkstemp
#include "ImageMesh.h"
#include "FileRangeBuilder.h"
#include "pcrecpp.h"

// libMesh includes
#include "libmesh/mesh_generation.h"

template<>
InputParameters validParams<ImageMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();
  params.addClassDescription("Generated mesh with the aspect ratio of a given image stack");

  // Add parameters associated with file ranges
  addFileRangeParams(params);

  // Add ImageMesh-specific params
  params.addParam<bool>("scale_to_one", true, "Whether or not to scale the image so its max dimension is 1");
  params.addRangeCheckedParam<Real>("cells_per_pixel", 1.0, "cells_per_pixel<=1.0", "The number of mesh cells per pixel, must be <=1 ");

  return params;
}

ImageMesh::ImageMesh(const std::string & name, InputParameters parameters) :
    GeneratedMesh(name, parameters),
    _scale_to_one(getParam<bool>("scale_to_one")),
    _cells_per_pixel(getParam<Real>("cells_per_pixel"))
{
  // Set up the parameters associated with file ranges
  int status = parseFileRange(_pars);

  // Failure is not an option
  if (status != 0)
    mooseError(getFileRangeErrorMessage(status));
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
  // Make sure the filenames parameter is valid.  This is set up by
  // the parseFileRange() function, see FileRangeBuilder.{C,h} for
  // more information.
  if (!isParamValid("filenames"))
    mooseError("ImageMesh Error: filenames parameter is not valid!");

  // Grab the list of filenames
  std::vector<std::string> filenames = getParam<std::vector<std::string> >("filenames");

  // A list of filenames of length 1 means we are building a 2D mesh
  if (filenames.size()==1)
    buildMesh2D(filenames[0]);

  else
    buildMesh3D(filenames);
}



void
ImageMesh::buildMesh3D(const std::vector<std::string> & filenames)
{
  // If the user gave us a "stack" with 0 or 1 files in it, we can't
  // really create a 3D Mesh from that
  if (filenames.size() <= 1)
    mooseError("ImageMesh error: Cannot create a 3D ImageMesh from an image stack with " << filenames.size() << " images.");

  // For each file in the stack, process it using the 'file' command.
  // We want to be sure that all the images in the stack are the same
  // size, for example...
  int
    xpixels = 0,
    ypixels = 0,
    zpixels = filenames.size();

  // Take pixel info from the first image in the stack to determine the aspect ratio
  GetPixelInfo(filenames[0], xpixels, ypixels);

  // TODO: Check that all images are the same aspect ratio and have
  // the same number of pixels?  ImageFunction does not currently do
  // this...
  // for (unsigned i=0; i<filenames.size(); ++i)
  // {
  //   // Extract the number of pixels from the image using the file command
  //   GetPixelInfo(filenames[i], xpixels, ypixels);
  //
  //   // Moose::out << "Image " << filenames[i] << " has size: " << xpixels << " by " << ypixels << std::endl;
  // }

  // Use the number of x and y pixels and the number of images to
  // determine the the x, y, and z dimensions of the mesh.  We assume
  // that there is 1 pixel in the z-direction for each image in the
  // stack.

  // Set the maximum dimension to 1.0 while scaling the other
  // directions to maintain the aspect ratio.
  Real
    xmax = xpixels,
    ymax = ypixels,
    zmax = zpixels;

  if (_scale_to_one)
  {
    Real max = std::max(std::max(xmax, ymax), zmax);
    xmax /= max;
    ymax /= max;
    zmax /= max;
  }

  // Compute the number of cells in the x and y direction based on
  // the user's cells_per_pixel parameter.  Note: we use ints here
  // because the GeneratedMesh params object uses ints for these...
  int
    nx = static_cast<int>(_cells_per_pixel * xpixels),
    ny = static_cast<int>(_cells_per_pixel * ypixels),
    nz = static_cast<int>(_cells_per_pixel * zpixels);

  // Actually build the Mesh
  MeshTools::Generation::build_cube(dynamic_cast<UnstructuredMesh&>(getMesh()),
                                    nx, ny, nz,
                                    /*xmin=*/0., /*xmax=*/xmax,
                                    /*ymin=*/0., /*ymax=*/ymax,
                                    /*zmin=*/0., /*zmax=*/zmax,
                                    HEX8);

  // We've determined the correct xmax, ymax, zmax values, so set them in
  // our InputParameters object.
  _pars.set<Real>("xmax") = xmax;
  _pars.set<Real>("ymax") = ymax;
  _pars.set<Real>("zmax") = zmax;

  // Set the number of cells in the x, y, and z directions that we determined.
  _pars.set<int>("nx") = nx;
  _pars.set<int>("ny") = ny;
  _pars.set<int>("nz") = nz;
}



void
ImageMesh::buildMesh2D(const std::string & filename)
{
  int
    xpixels = 0,
    ypixels = 0;

  // Extract the number of pixels from the image using the file command
  GetPixelInfo(filename, xpixels, ypixels);

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
    // sixteenth_image001_cropped3_closing_298.png: PNG image data, 115 x 99, 16-bit/color RGB, non-interlaced
    xpixels = 0, ypixels = 0;
    pcrecpp::RE re("(\\d+) x (\\d+)");
    re.PartialMatch(command_result, &xpixels, &ypixels);

    // Detect failure of the regex
    if ((xpixels==0) || (ypixels==0))
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

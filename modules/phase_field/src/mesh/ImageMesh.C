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
#include "tinydir.h"

// libMesh includes
#include "libmesh/mesh_generation.h"

template<>
InputParameters validParams<ImageMesh>()
{
  InputParameters params = validParams<GeneratedMesh>();
  params.addParam<std::string>("file", "Name of single image file to extract mesh parameters from.  If provided, a 2D mesh is created.");
  params.addParam<std::string>("file_base", "Image file base to open, use this option when a stack of images must be read (ignored if 'file' is given)");
  params.addParam<std::vector<unsigned int> >("file_range", "Range of images to analyze, used with 'file_base' (ignored if 'file' is given)");
  params.addParam<std::string>("file_suffix", "Suffix of the file to open, e.g. 'png'");
  params.addParam<bool>("scale_to_one", true, "Whether or not to scale the image so its max dimension is 1");
  params.addRangeCheckedParam<Real>("cells_per_pixel", 1.0, "cells_per_pixel<=1.0", "The number of mesh cells per pixel, must be <=1 ");
  return params;
}

ImageMesh::ImageMesh(const std::string & name, InputParameters parameters) :
    GeneratedMesh(name, parameters),
    _has_file(isParamValid("file")),
    _file(_has_file ? getParam<std::string>("file") : ""),
    _has_file_base(isParamValid("file_base")),
    _file_base(_has_file_base ? getParam<std::string>("file_base") : ""),
    _has_file_range(isParamValid("file_range")),
    _file_range(_has_file_range ? getParam<std::vector<unsigned int> >("file_range") : std::vector<unsigned int>()),
    _has_file_suffix(isParamValid("file_suffix")),
    _file_suffix(_has_file_suffix ? getParam<std::string>("file_suffix") : ""),
    _scale_to_one(getParam<bool>("scale_to_one")),
    _cells_per_pixel(getParam<Real>("cells_per_pixel"))
{
  // If the user provides one of the following combinations:
  // .) file+file_base
  // .) no file+no file_base
  // .) file_base+no file_suffix
  // then the input file is ambiguous and we throw an error.
  if (_has_file && _has_file_base)
    mooseError("ImageMesh error: cannot provide both file = " << _file << " and file_base = " << _file_base);

  if (!_has_file && !_has_file_base)
    mooseError("ImageMesh error: You must provide a valid value for either the 'file' parameter or the 'file_base' parameter.");

  if (_has_file_base && !_has_file_suffix)
    mooseError("ImageMesh error: If you provide a 'file_base', you must also provide a valid 'file_suffix', e.g. 'png'.");

  // If the user provided a filename and a range, warn that the range will be ignored.
  if (_has_file && _has_file_range)
    mooseWarning("Warning: file_range was ignored since a filename was provided.");

  // If the user provided a file_base but not a file_range, we'll create one
  if (_has_file_base && !_has_file_range)
  {
    _file_range.push_back(0);
    _file_range.push_back(std::numeric_limits<unsigned int>::max());
  }

  // If the user provided a file_range with a single entry, repeat it
  // for them.  This signifies a single image (i.e. 2D Mesh) is to be
  // used.
  if (_has_file_range && _file_range.size() == 1)
    _file_range.push_back(_file_range[0]);

  // If the user provided a file_range with too many entries, print a
  // warning and truncate the extra values.
  if (_has_file_range && _file_range.size() != 2)
  {
    mooseWarning("A maximum of two values are allowed in the _file_range, extra values truncated.");
    _file_range.resize(2);
  }

  // Make sure that the range is in ascending order
  std::sort(_file_range.begin(), _file_range.end());
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
  if (_has_file)
    buildMesh2D();

  else if (_has_file_base)
    buildMesh3D();

  else
    mooseError("We'll never get here!");
}



void
ImageMesh::buildMesh3D()
{
  // 1.) Build up a list of filenames which comprise the stack using tinydir and pcrecpp

  // Separate the file base from the path
  std::pair<std::string, std::string> split_file = MooseUtils::splitFileName(_file_base);

  // Create directory object
  tinydir_dir dir;
  tinydir_open_sorted(&dir, split_file.first.c_str());

  // This regex will capture the file_base and any number of digits when used with FullMatch()
  std::ostringstream oss;
  oss << "(" << split_file.second << ".*?(\\d+))\\..*";
  pcrecpp::RE file_base_and_num_regex(oss.str());

  // Loop through the files in the directory
  for (int i = 0; i < dir.n_files; i++)
  {
    // Update the current file
    tinydir_file file;
    tinydir_readfile_n(&dir, &file, i);

    // Store the file if it has proper extension as in numeric range
    if (!file.is_dir && MooseUtils::hasExtension(file.name, _file_suffix))
    {
      std::string the_base;
      unsigned int file_num = 0;
      file_base_and_num_regex.FullMatch(file.name, &the_base, &file_num);

      if (!the_base.empty() && file_num >= _file_range[0] && file_num <= _file_range[1])
        _stack_filenames.push_back(split_file.first + "/" + file.name);
    }
  }
  tinydir_close(&dir);

  // If the user gave us a "stack" with 0 or 1 files in it, we can't
  // really create a 3D Mesh from that
  if (_stack_filenames.size() <= 1)
    mooseError("ImageMesh error: Cannot create a 3D ImageMesh from an image stack with " << _stack_filenames.size() << " images.");

  // For each file in the stack, process it using the 'file' command.
  // We want to be sure that all the images in the stack are the same
  // size, for example...
  int
    xpixels = 0,
    ypixels = 0,
    zpixels = _stack_filenames.size();

  // Take pixel info from the first image in the stack to determine the aspect ratio
  GetPixelInfo(_stack_filenames[0], xpixels, ypixels);

  // TODO: Check that all images are the same aspect ratio and have
  // the same number of pixels?  ImageFunction does not currently do
  // this...
  // for (unsigned i=0; i<_stack_filenames.size(); ++i)
  // {
  //   // Extract the number of pixels from the image using the file command
  //   GetPixelInfo(_stack_filenames[i], xpixels, ypixels);
  //
  //   // Moose::out << "Image " << _stack_filenames[i] << " has size: " << xpixels << " by " << ypixels << std::endl;
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
ImageMesh::buildMesh2D()
{
  int
    xpixels = 0,
    ypixels = 0;

  // Extract the number of pixels from the image using the file command
  GetPixelInfo(_file, xpixels, ypixels);

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

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

#include "ImageFunction.h"
#include "MooseUtils.h"

// External includes
#include "pcrecpp.h"
#include "tinydir.h"

template<>
InputParameters validParams<ImageFunction>()
{
  // Define the possible image formats
  MooseEnum type("png, tif, tiff", "png");

  // Define the general parameters
  InputParameters params = validParams<Function>();
  params.addParam<FileName>("file", "Image to open, utilize this option when a single file is given");
  params.addParam<FileName>("file_base", "Image file base to open, use this option when a stack of images must be read (ignord if 'file' is given)");
  params.addParam<MooseEnum>("file_type", type, "Image file type, use this to specify the type of file for an image stack (use with 'file_base'; ignored if 'file' is given)");
  params.addParam<std::vector<unsigned int> >("file_range", "Range of images to analyze, used with 'file_base' (ignored if 'file' is given)");
  params.addParam<std::vector<Real> >("origin", "Origin of the image (defualts to mesh origin)");
  params.addParam<std::vector<Real> >("dimensions", "x,y,z dimensions of the image (defaults to mesh dimensions)");
  params.addParam<unsigned int>("component", "The image component to return, leaving this blank will result in a greyscale value for the image to be created. The component number is zero based, i.e. 0 returns the first component of the image");

  // Shift and Scale (application of these occurs prior to threshold)
  params.addParam<double>("shift", 0, "Value to add to all pixels; occurs prior to scaling");
  params.addParam<double>("scale", 1, "Multiplier to apply to all pixel values; occurs after shifting");
  params.addParamNamesToGroup("shift scale", "Rescale");

  // Threshold parameters
  params.addParam<double>("threshold", "The threshold value");
  params.addParam<double>("upper_value", "The value to set for data greater than the threshold value");
  params.addParam<double>("lower_value", "The value to set for data less than the threshold value");
  params.addParamNamesToGroup("threshold upper_value lower_value", "Threshold");

  return params;
}

ImageFunction::ImageFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters),
#ifdef LIBMESH_HAVE_VTK
    _data(NULL),
#endif
    _file_base(getParam<FileName>("file_base")),
    _file_type(getParam<MooseEnum>("file_type")),
    _origin(getParam<std::vector<Real> >("origin"))
{
#ifndef LIBMESH_HAVE_VTK
  // This should be impossible to reach, the registration of ImageFunction is also guarded with LIBMESH_HAVE_VTK
  mooseError("libMesh must be configured with VTK enabled to utilize ImageFunction");
#endif
}

void
ImageFunction::initialSetup()
{
#ifdef LIBMESH_HAVE_VTK
  // Initialize the image data (i.e, set origin, ...)
  initImageData();

  // Create a list of files to extract data from
  getFiles();

  // Read the image stack
  if (_file_type == "png")
    readImages<vtkPNGReader>();
  else if (_file_type == "tiff" || _file_type == "tif")
    readImages<vtkTIFFReader>();
  else
    mooseError("Un-supported file type '" << _file_type << "'");

  // Set the component parameter
  /* If the parameter is not set then vtkMagnitude() will applied */
  if (isParamValid("component"))
  {
    unsigned int n = _data->GetNumberOfScalarComponents();
    _component = getParam<unsigned int>("component");
    if (_component >= n)
      mooseError("'component' parameter must be empty or have a value of 0 to " << n-1);
  }
  else
    _component = 0;

  // Apply filters (only applied if appropriate, this is handled internally by each method)
  vtkMagnitude();
  vtkShiftAndScale();
  vtkThreshold();
#endif
}

ImageFunction::~ImageFunction()
{
}

Real
ImageFunction::value(Real /*t*/, const Point & p)
{
#ifdef LIBMESH_HAVE_VTK
  // Compute the pixel coordinates
  int x = std::floor(p(0)/_voxel[0]);
  int y = std::floor(p(1)/_voxel[1]);
  int z = std::floor(p(2)/_voxel[2]);

  // If the point falls on the mesh extents the index needs to be decreased by one
  if (x == _dims[0])
    x--;
  if (y == _dims[1])
    y--;
  if (z == _dims[2])
    z--;

  // Return the image data at the given point
  return _data->GetScalarComponentAsDouble(x, y, z, _component);
#else
  libmesh_ignore(p); // avoid un-used parameter warnings
  return 0.0;
#endif
}

void
ImageFunction::initImageData()
{
#ifdef LIBMESH_HAVE_VTK
  // Get access to the Mesh object
  FEProblem * fe_problem = getParam<FEProblem *>("_fe_problem");
  MooseMesh & mesh = fe_problem->mesh();

  // Set the dimensions from the Mesh if not set by the User
  if (isParamValid("dimensions"))
    _physical_dims = getParam<std::vector<Real> >("dimensions");

  else
  {
    _physical_dims.push_back(mesh.getParam<Real>("xmax") - mesh.getParam<Real>("xmin"));
    _physical_dims.push_back(mesh.getParam<Real>("ymax") - mesh.getParam<Real>("ymin"));
    _physical_dims.push_back(mesh.getParam<Real>("zmax") - mesh.getParam<Real>("zmin"));
  }

  // Set the origin from the Mesh if not set in the input file
  if (isParamValid("origin"))
    _origin = getParam<std::vector<Real> >("origin");

  else
  {
    _origin.push_back(mesh.getParam<Real>("xmin"));
    _origin.push_back(mesh.getParam<Real>("ymin"));
    _origin.push_back(mesh.getParam<Real>("zmin"));
  }

  // Check the file range and do some error checking
  if (isParamValid("file_range"))
  {
    _file_range = getParam<std::vector<unsigned int> >("file_range");

    if (_file_range.size() == 1)
      _file_range.push_back(_file_range[0]);

    if (_file_range.size() != 2)
      mooseError("Image range must specify one or two interger values");

    if (_file_range[1] < _file_range[0])
      mooseError("Image range must specify exactly two interger values, with the second larger than the first");
  }
  else
  {
    _file_range.push_back(0);
    _file_range.push_back(std::numeric_limits<unsigned int>::max());
  }
#endif
}

void
ImageFunction::getFiles()
{
#ifdef LIBMESH_HAVE_VTK
  // Storage for the file names
  _files = vtkSmartPointer<vtkStringArray>::New();

  // Use specified file name
  if (isParamValid("file"))
  {
    std::string filename = getParam<FileName>("file");
    _files->InsertNextValue(filename);
    _file_type = filename.substr(filename.find_last_of(".") + 1);
  }

  // File stack
  else
  {
    // Separate the file base from the path
    std::pair<std::string, std::string> split_file = MooseUtils::splitFileName(_file_base);

    // Create directory object
    tinydir_dir dir;
    tinydir_open_sorted(&dir, split_file.first.c_str());

    // Regex for extracting numbers from file
    std::ostringstream oss;
    oss << "(" << split_file.second << ".*?(\\d+))\\..*";
    pcrecpp::RE re_base_and_file_num(oss.str()); // Will pull out the full base and the file number simultaneously

    // Loop through the files in the directory
    for (unsigned int i = 0; i < dir.n_files; i++)
    {
      // Upate the current file
      tinydir_file file;
      tinydir_readfile_n(&dir, &file, i);

      // Store the file if it has proper extension as in numeric range
      if (!file.is_dir && MooseUtils::hasExtension(file.name, _file_type))
      {
        std::string the_base;
        int file_num = 0;
        re_base_and_file_num.FullMatch(file.name, &the_base, &file_num);
        if (!the_base.empty() && file_num >= _file_range[0] && file_num <= _file_range[1])
          _files->InsertNextValue(split_file.first + "/" + file.name);
      }
    }
    tinydir_close(&dir);
  }

  // Error if no files where located
  if (_files->GetNumberOfValues() == 0)
    mooseError("No image file(s) located");
#endif
}

void
ImageFunction::vtkMagnitude()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if component is set
  if (isParamValid("component"))
    return;

  // Apply the greyscale filtering
  _magnitude_filter = vtkSmartPointer<vtkImageMagnitude>::New();
#if VTK_MAJOR_VERSION <= 5
  _magnitude_filter->SetInput(_data);
#else
  _magnitude_filter->SetInputData(_data);
#endif
  _magnitude_filter->Update();

  _data = _magnitude_filter->GetOutput();
#endif
}

void
ImageFunction::vtkShiftAndScale()
{
#ifdef LIBMESH_HAVE_VTK
  // Extract the parameters
  double shift = getParam<double>("shift");
  double scale = getParam<double>("scale");

  // Do nothing if shift and scale are not set beyond defaults
  if (shift == 0 && scale == 1)
    return;

  // Perform the scaling and offset actions
  _shift_scale_filter = vtkSmartPointer<vtkImageShiftScale>::New();
  _shift_scale_filter->SetOutputScalarTypeToDouble();

#if VTK_MAJOR_VERSION <= 5
  _shift_scale_filter->SetInput(_data);
#else
  _shift_scale_filter->SetInputData(_data);
#endif
  _shift_scale_filter->SetShift(shift);
  _shift_scale_filter->SetScale(scale);
  _shift_scale_filter->Update();
  _data = _shift_scale_filter->GetOutput();
#endif
}

void
ImageFunction::vtkThreshold()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if the threshold parameter is not set
  if (!isParamValid("threshold"))
    return;

  // Error if both upper and lower are not set
  if (!isParamValid("upper_value") || !isParamValid("lower_value"))
    mooseError("When thresholding is applied, both the upper_value and lower_value parameters must be set");

  // Create the thresholding object
  _image_threshold = vtkSmartPointer<vtkImageThreshold>::New();

  // Set the data source
#if VTK_MAJOR_VERSION < 6
  _image_threshold->SetInput(_data);
#else
  _image_threshold->SetInputData(_data);
#endif

  // Setup the thresholding options
  _image_threshold->ThresholdByUpper(getParam<Real>("threshold"));
  _image_threshold->ReplaceInOn();
  _image_threshold->SetInValue(getParam<Real>("upper_value"));
  _image_threshold->ReplaceOutOn();
  _image_threshold->SetOutValue(getParam<Real>("lower_value"));
  _image_threshold->SetOutputScalarTypeToDouble();

  // Perform the thresholding
  _image_threshold->Update();
  _data = _image_threshold->GetOutput();
#endif
}

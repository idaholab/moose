//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ImageSampler.h"
#include "MooseApp.h"
#include "ImageMesh.h"

#include "libmesh/mesh_tools.h"

using namespace libMesh;

InputParameters
ImageSampler::validParams()
{
  // Define the general parameters
  InputParameters params = emptyInputParameters();
  params += FileRangeBuilder::validParams();

  params.addParam<Point>("origin", "Origin of the image (defaults to mesh origin)");
  params.addParam<Point>("dimensions",
                         "x,y,z dimensions of the image (defaults to mesh dimensions)");
  params.addParam<unsigned int>(
      "component",
      "The image RGB-component to return, leaving this blank will result in a greyscale value "
      "for the image to be created. The component number is zero based, i.e. 0 returns the first "
      "(RED) component of the image.");

  // Shift and Scale (application of these occurs prior to threshold)
  params.addParam<double>("shift", 0, "Value to add to all pixels; occurs prior to scaling");
  params.addParam<double>(
      "scale", 1, "Multiplier to apply to all pixel values; occurs after shifting");
  params.addParamNamesToGroup("shift scale", "Rescale");

  // Threshold parameters
  params.addParam<double>("threshold", "The threshold value");
  params.addParam<double>(
      "upper_value", 1, "The value to set for data greater than the threshold value");
  params.addParam<double>(
      "lower_value", 0, "The value to set for data less than the threshold value");
  params.addParamNamesToGroup("threshold upper_value lower_value", "Threshold");

  // Flip image
  params.addParam<bool>("flip_x", false, "Flip the image along the x-axis");
  params.addParam<bool>("flip_y", false, "Flip the image along the y-axis");
  params.addParam<bool>("flip_z", false, "Flip the image along the z-axis");
  params.addParamNamesToGroup("flip_x flip_y flip_z", "Flip");

  return params;
}

ImageSampler::ImageSampler(const InputParameters & parameters)
  : FileRangeBuilder(parameters),
#ifdef LIBMESH_HAVE_VTK
    _data(nullptr),
    _algorithm(nullptr),
#endif
    _is_pars(parameters),
    _is_console(
        (parameters.getCheckedPointerParam<MooseApp *>("_moose_app"))->getOutputWarehouse()),
    _flip({{_is_pars.get<bool>("flip_x"),
            _is_pars.get<bool>("flip_y"),
            _is_pars.get<bool>("flip_z")}})

{
#ifndef LIBMESH_HAVE_VTK
  // This should be impossible to reach, the registration of ImageSampler is also guarded with
  // LIBMESH_HAVE_VTK
  mooseError("libMesh must be configured with VTK enabled to utilize ImageSampler");
#endif
}

void
ImageSampler::setupImageSampler(MooseMesh & mesh)
{
  // Don't warn that mesh or _is_pars are unused when VTK is not enabled.
  libmesh_ignore(mesh);
  libmesh_ignore(_is_pars);

#ifdef LIBMESH_HAVE_VTK
  // Get access to the Mesh object
  BoundingBox bbox = MeshTools::create_bounding_box(mesh.getMesh());

  // Set the dimensions from the Mesh if not set by the User
  if (_is_pars.isParamValid("dimensions"))
    _physical_dims = _is_pars.get<Point>("dimensions");

  else
  {
    _physical_dims(0) = bbox.max()(0) - bbox.min()(0);
#if LIBMESH_DIM > 1
    _physical_dims(1) = bbox.max()(1) - bbox.min()(1);
#endif
#if LIBMESH_DIM > 2
    _physical_dims(2) = bbox.max()(2) - bbox.min()(2);
#endif
  }

  // Set the origin from the Mesh if not set in the input file
  if (_is_pars.isParamValid("origin"))
    _origin = _is_pars.get<Point>("origin");
  else
  {
    _origin(0) = bbox.min()(0);
#if LIBMESH_DIM > 1
    _origin(1) = bbox.min()(1);
#endif
#if LIBMESH_DIM > 2
    _origin(2) = bbox.min()(2);
#endif
  }

  // An array of filenames, to be filled in
  std::vector<std::string> filenames;

  // The file suffix, to be determined
  std::string file_suffix;

  // Try to parse our own file range parameters.  If that fails, then
  // see if the associated Mesh is an ImageMesh and use its.  If that
  // also fails, then we have to throw an error...
  //
  // The parseFileRange method sets parameters, thus a writable reference to the InputParameters
  // object must be obtained from the warehouse. Generally, this should be avoided, but
  // this is a special case.
  if (_status != 0)
  {
    // We don't have parameters, so see if we can get them from ImageMesh
    ImageMesh * image_mesh = dynamic_cast<ImageMesh *>(&mesh);
    if (!image_mesh)
      mooseError("No file range parameters were provided and the Mesh is not an ImageMesh.");

    // Get the ImageMesh's parameters.  This should work, otherwise
    // errors would already have been thrown...
    filenames = image_mesh->filenames();
    file_suffix = image_mesh->fileSuffix();
  }
  else
  {
    // Use our own parameters (using 'this' b/c of conflicts with filenames the local variable)
    filenames = this->filenames();
    file_suffix = fileSuffix();
  }

  // Storage for the file names
  _files = vtkSmartPointer<vtkStringArray>::New();

  for (const auto & filename : filenames)
    _files->InsertNextValue(filename);

  // Error if no files where located
  if (_files->GetNumberOfValues() == 0)
    mooseError("No image file(s) located");

  // Read the image stack.  Hurray for VTK not using polymorphism in a
  // smart way... we actually have to explicitly create the type of
  // reader based on the file extension, using an if-statement...
  if (file_suffix == "png")
    _image = vtkSmartPointer<vtkPNGReader>::New();
  else if (file_suffix == "tiff" || file_suffix == "tif")
    _image = vtkSmartPointer<vtkTIFFReader>::New();
  else
    mooseError("Un-supported file type '", file_suffix, "'");

  // Now that _image is set up, actually read the images
  // Indicate that data read has started
  _is_console << "Reading image(s)..." << std::endl;

  // Extract the data
  _image->SetFileNames(_files);
  _image->Update();
  _data = _image->GetOutput();
  _algorithm = _image->GetOutputPort();

  // Set the image dimensions and voxel size member variable
  int * dims = _data->GetDimensions();
  for (unsigned int i = 0; i < 3; ++i)
  {
    _dims.push_back(dims[i]);
    _voxel.push_back(_physical_dims(i) / _dims[i]);
  }

  // Set the dimensions of the image and bounding box
  _data->SetSpacing(_voxel[0], _voxel[1], _voxel[2]);
  _data->SetOrigin(_origin(0), _origin(1), _origin(2));
  _bounding_box.min() = _origin;
  _bounding_box.max() = _origin + _physical_dims;

  // Indicate data read is completed
  _is_console << "          ...image read finished" << std::endl;

  // Set the component parameter
  // If the parameter is not set then vtkMagnitude() will applied
  if (_is_pars.isParamValid("component"))
  {
    unsigned int n = _data->GetNumberOfScalarComponents();
    _component = _is_pars.get<unsigned int>("component");
    if (_component >= n)
      mooseError("'component' parameter must be empty or have a value of 0 to ", n - 1);
  }
  else
    _component = 0;

  // Apply filters, the toggling on and off of each filter is handled internally
  vtkMagnitude();
  vtkShiftAndScale();
  vtkThreshold();
#endif
}

Real
ImageSampler::sample(const Point & p) const
{
#ifdef LIBMESH_HAVE_VTK

  // Do nothing if the point is outside of the image domain
  if (!_bounding_box.contains_point(p))
    return 0.0;

  // Determine pixel coordinates
  std::vector<int> x(3, 0);
  for (int i = 0; i < LIBMESH_DIM; ++i)
  {
    // Compute position, only if voxel size is greater than zero
    if (_voxel[i] != 0)
    {
      x[i] = std::floor((p(i) - _origin(i)) / _voxel[i]);

      // If the point falls on the mesh extents the index needs to be decreased by one
      if (x[i] == _dims[i])
        x[i]--;

      // flip
      if (_flip[i])
        x[i] = _dims[i] - x[i] - 1;
    }
  }

  // Return the image data at the given point
  return _data->GetScalarComponentAsDouble(x[0], x[1], x[2], _component);

#else
  libmesh_ignore(p); // avoid un-used parameter warnings
  return 0.0;
#endif
}

void
ImageSampler::vtkMagnitude()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if 'component' is set
  if (_is_pars.isParamValid("component"))
    return;

  // Apply the greyscale filtering
  _magnitude_filter = vtkSmartPointer<vtkImageMagnitude>::New();
  _magnitude_filter->SetInputConnection(_algorithm);
  _magnitude_filter->Update();

  // Update the pointers
  _data = _magnitude_filter->GetOutput();
  _algorithm = _magnitude_filter->GetOutputPort();
#endif
}

void
ImageSampler::vtkShiftAndScale()
{
#ifdef LIBMESH_HAVE_VTK
  // Capture the parameters
  double shift = _is_pars.get<double>("shift");
  double scale = _is_pars.get<double>("scale");

  // Do nothing if shift and scale are not set
  if (shift == 0 && scale == 1)
    return;

  // Perform the scaling and offset actions
  _shift_scale_filter = vtkSmartPointer<vtkImageShiftScale>::New();
  _shift_scale_filter->SetOutputScalarTypeToDouble();

  _shift_scale_filter->SetInputConnection(_algorithm);
  _shift_scale_filter->SetShift(shift);
  _shift_scale_filter->SetScale(scale);
  _shift_scale_filter->Update();

  // Update the pointers
  _data = _shift_scale_filter->GetOutput();
  _algorithm = _shift_scale_filter->GetOutputPort();
#endif
}

void
ImageSampler::vtkThreshold()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if threshold not set
  if (!_is_pars.isParamValid("threshold"))
    return;

  // Error if both upper and lower are not set
  if (!_is_pars.isParamValid("upper_value") || !_is_pars.isParamValid("lower_value"))
    mooseError("When thresholding is applied, both the upper_value and lower_value parameters must "
               "be set");

  // Create the thresholding object
  _image_threshold = vtkSmartPointer<vtkImageThreshold>::New();

  // Set the data source
  _image_threshold->SetInputConnection(_algorithm);

  // Setup the thresholding options
  _image_threshold->ThresholdByUpper(_is_pars.get<Real>("threshold"));
  _image_threshold->ReplaceInOn();
  _image_threshold->SetInValue(_is_pars.get<Real>("upper_value"));
  _image_threshold->ReplaceOutOn();
  _image_threshold->SetOutValue(_is_pars.get<Real>("lower_value"));
  _image_threshold->SetOutputScalarTypeToDouble();

  // Perform the thresholding
  _image_threshold->Update();

  // Update the pointers
  _data = _image_threshold->GetOutput();
  _algorithm = _image_threshold->GetOutputPort();
#endif
}

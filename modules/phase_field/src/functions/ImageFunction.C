/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ImageFunction.h"
#include "MooseUtils.h"
#include "FileRangeBuilder.h"
#include "ImageMesh.h"

template<>
InputParameters validParams<ImageFunction>()
{
  // Define the general parameters
  InputParameters params = validParams<Function>();
  params.addClassDescription("Function with values sampled from a given image stack");

  // Add parameters associated with file ranges
  addFileRangeParams(params);

  params.addParam<Point>("origin", "Origin of the image (defaults to mesh origin)");
  params.addParam<Point>("dimensions", "x,y,z dimensions of the image (defaults to mesh dimensions)");
  params.addParam<unsigned int>("component", "The image component to return, leaving this blank will result in a greyscale value "
                                             "for the image to be created. The component number is zero based, i.e. 0 returns the first component of the image");

  // Shift and Scale (application of these occurs prior to threshold)
  params.addParam<double>("shift", 0, "Value to add to all pixels; occurs prior to scaling");
  params.addParam<double>("scale", 1, "Multiplier to apply to all pixel values; occurs after shifting");
  params.addParamNamesToGroup("shift scale", "Rescale");

  // Threshold parameters
  params.addParam<double>("threshold", "The threshold value");
  params.addParam<double>("upper_value", "The value to set for data greater than the threshold value");
  params.addParam<double>("lower_value", "The value to set for data less than the threshold value");
  params.addParamNamesToGroup("threshold upper_value lower_value", "Threshold");

  // Flip image
  params.addParam<bool>("flip_x", false, "Flip the image along the x-axis");
  params.addParam<bool>("flip_y", false, "Flip the image along the y-axis");
  params.addParam<bool>("flip_z", false, "Flip the image along the z-axis");
  params.addParamNamesToGroup("flip_x flip_y flip_z", "Flip");

  return params;
}

ImageFunction::ImageFunction(const std::string & name, InputParameters parameters) :
    Function(name, parameters)
#ifdef LIBMESH_HAVE_VTK
    ,_data(NULL)
    ,_algorithm(NULL)
#endif
{
#ifndef LIBMESH_HAVE_VTK
  // This should be impossible to reach, the registration of ImageFunction is also guarded with LIBMESH_HAVE_VTK
  mooseError("libMesh must be configured with VTK enabled to utilize ImageFunction");
#endif
}

ImageFunction::~ImageFunction()
{
}

void
ImageFunction::initialSetup()
{
#ifdef LIBMESH_HAVE_VTK
  // Get access to the Mesh object
  FEProblem * fe_problem = getParam<FEProblem *>("_fe_problem");
  MooseMesh & mesh = fe_problem->mesh();

  // Set the dimensions from the Mesh if not set by the User
  if (isParamValid("dimensions"))
    _physical_dims = getParam<Point>("dimensions");

  else
  {
    _physical_dims(0) = mesh.getParam<Real>("xmax") - mesh.getParam<Real>("xmin");
#if LIBMESH_DIM > 1
    _physical_dims(1) = mesh.getParam<Real>("ymax") - mesh.getParam<Real>("ymin");
#endif
#if LIBMESH_DIM > 2
    _physical_dims(2) = mesh.getParam<Real>("zmax") - mesh.getParam<Real>("zmin");
#endif
  }

  // Set the origin from the Mesh if not set in the input file
  if (isParamValid("origin"))
    _origin = getParam<Point>("origin");
  else
  {
    _origin(0) = mesh.getParam<Real>("xmin");
#if LIBMESH_DIM > 1
    _origin(1) = mesh.getParam<Real>("ymin");
#endif
#if LIBMESH_DIM > 2
    _origin(2) = mesh.getParam<Real>("zmin");
#endif
  }


  // An array of filenames, to be filled in
  std::vector<std::string> filenames;

  // The file suffix, to be determined
  std::string file_suffix;

  // Try to parse our own file range parameters.  If that fails, then
  // see if the associated Mesh is an ImageMesh and use its.  If that
  // also fails, then we have to throw an error...
  int status = parseFileRange(_pars);

  if (status != 0)
  {
    // We don't have parameters, so see if we can get them from ImageMesh
    ImageMesh * image_mesh = dynamic_cast<ImageMesh*>(&mesh);
    if (!image_mesh)
      mooseError("No file range parameters were provided and the Mesh is not an ImageMesh.");

    // Get the ImageMesh's parameters.  This should work, otherwise
    // errors would already have been thrown...
    InputParameters & im_params = image_mesh->parameters();
    filenames = im_params.get<std::vector<std::string> >("filenames");
    file_suffix = im_params.get<std::string>("file_suffix");
  }
  else
  {
    // Use our own parameters
    filenames = getParam<std::vector<std::string> >("filenames");
    file_suffix = getParam<std::string>("file_suffix");
  }

  // Storage for the file names
  _files = vtkSmartPointer<vtkStringArray>::New();

  for (unsigned i=0; i<filenames.size(); ++i)
    _files->InsertNextValue(filenames[i]);

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
    mooseError("Un-supported file type '" << file_suffix << "'");

  // Now that _image is set up, actually read the images
  // Indicate that data read has started
  _console << "Reading image(s)..." << std::endl;

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
    _voxel.push_back(_physical_dims(i)/_dims[i]);
  }

  // Set the dimensions of the image and bounding box
  _data->SetSpacing(_voxel[0], _voxel[1], _voxel[2]);
  _data->SetOrigin(_origin(0), _origin(1), _origin(2));
  _bounding_box.min() = _origin;
  _bounding_box.max() = _origin + _physical_dims;

  // Indicate data read is completed
  _console << "          ...image read finished" << std::endl;

  // Set the component parameter
  // If the parameter is not set then vtkMagnitude() will applied
  if (isParamValid("component"))
  {
    unsigned int n = _data->GetNumberOfScalarComponents();
    _component = getParam<unsigned int>("component");
    if (_component >= n)
      mooseError("'component' parameter must be empty or have a value of 0 to " << n-1);
  }
  else
    _component = 0;

  // Apply filters, the toggling on and off of each filter is handled internally
  vtkMagnitude();
  vtkShiftAndScale();
  vtkThreshold();
  vtkFlip();
#endif
}

Real
ImageFunction::value(Real /*t*/, const Point & p)
{
#ifdef LIBMESH_HAVE_VTK

  // Do nothing if the point is outside of the image domain
  if (!_bounding_box.contains_point(p))
    return 0.0;

  // Determine pixel coordinates
  std::vector<int> x(3,0);
  for (int i = 0; i < LIBMESH_DIM; ++i)
  {
    // Compute position, only if voxel size is greater than zero
    if (_voxel[i] == 0)
      x[i] = 0;

    else
    {
      x[i] = std::floor((p(i) - _origin(i))/_voxel[i]);

      // If the point falls on the mesh extents the index needs to be decreased by one
      if (x[i] == _dims[i])
        x[i]--;
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
ImageFunction::vtkMagnitude()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if 'component' is set
  if (isParamValid("component"))
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
ImageFunction::vtkShiftAndScale()
{
#ifdef LIBMESH_HAVE_VTK
  // Capture the parameters
  double shift = getParam<double>("shift");
  double scale = getParam<double>("scale");

  // Do nothing if shift and scale are not set
  if (shift == 0 || scale == 1)
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
ImageFunction::vtkThreshold()
{
#ifdef LIBMESH_HAVE_VTK
  // Do nothing if threshold not set
  if (!isParamValid("threshold"))
    return;

  // Error if both upper and lower are not set
  if (!isParamValid("upper_value") || !isParamValid("lower_value"))
    mooseError("When thresholding is applied, both the upper_value and lower_value parameters must be set");

  // Create the thresholding object
  _image_threshold = vtkSmartPointer<vtkImageThreshold>::New();

  // Set the data source
  _image_threshold->SetInputConnection(_algorithm);

  // Setup the thresholding options
  _image_threshold->ThresholdByUpper(getParam<Real>("threshold"));
  _image_threshold->ReplaceInOn();
  _image_threshold->SetInValue(getParam<Real>("upper_value"));
  _image_threshold->ReplaceOutOn();
  _image_threshold->SetOutValue(getParam<Real>("lower_value"));
  _image_threshold->SetOutputScalarTypeToDouble();

  // Perform the thresholding
  _image_threshold->Update();

  // Update the pointers
  _data = _image_threshold->GetOutput();
  _algorithm = _image_threshold->GetOutputPort();
#endif
}

void
ImageFunction::vtkFlip()
{
#ifdef LIBMESH_HAVE_VTK
  // Convert boolean values into an integer array, then loop over it
  int mask[3] = {getParam<bool>("flip_x"),
                 getParam<bool>("flip_y"),
                 getParam<bool>("flip_z")};

  for (int dim=0; dim<3; ++dim)
  {
    if (mask[dim])
    {
      _flip_filter = imageFlip(dim);

      // Update pointers
      _data = _flip_filter->GetOutput();
      _algorithm = _flip_filter->GetOutputPort();
    }
  }
#endif
}

#ifdef LIBMESH_HAVE_VTK
vtkSmartPointer<vtkImageFlip>
ImageFunction::imageFlip(const int & axis)
{
  vtkSmartPointer<vtkImageFlip> flip_image = vtkSmartPointer<vtkImageFlip>::New();

  flip_image->SetFilteredAxis(axis);

  // Set the data source
  flip_image->SetInputConnection(_algorithm);

  // Perform the flip
  flip_image->Update();

  // Return the flip filter pointer
  return flip_image;
}
#endif

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

#ifndef IMAGEFUNCTION_H
#define IMAGEFUNCTION_H

// MOOSE includes
#include "Function.h"

// VTK includes
#ifdef LIBMESH_HAVE_VTK
#include "vtkSmartPointer.h"
#include "vtkPNGReader.h"
#include "vtkTIFFReader.h"
#include "vtkImageData.h"
#include "vtkStringArray.h"
#include "vtkImageThreshold.h"
#include "vtkImageNormalize.h"
#include "vtkImageCast.h"
#include "vtkImageShiftScale.h"
#include "vtkImageMagnitude.h"
#endif

// Forward declarations
class ImageFunction;

template<>
InputParameters validParams<ImageFunction>();

/**
 * A function for extracting data from an image or stack of images
 */
class ImageFunction : public Function
{
public:

  /**
   * Class constructor
   * @param name
   * @param parameters
   */
  ImageFunction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~ImageFunction();

  /**
   * Return the pixel value for the given point
   * @param t Time (unused)
   * @param p The point at which to extract pixel data
   */
  virtual Real value(Real t, const Point & p);

  /**
   * Perform initialization of image data
   */
  virtual void initialSetup();

protected:

  /**
   * Apply image re-scaling using the vtkImageShiftAndRescale object
   */
  void vtkShiftAndScale();

  /**
   * Perform thresholding
   */
  void vtkThreshold();

  /**
   * Convert the image to greyscale
   *
   * By leaving the 'component' input parameter empty, this is called automatically.
   */
  void vtkMagnitude();

#ifdef LIBMESH_HAVE_VTK

  /// List of file names to extract data
  vtkSmartPointer<vtkStringArray> _files;

  /// Complete image data
  vtkImageData * _data;

  /// Complete image data
  vtkSmartPointer<vtkImageReader2> _image;

  /// Pointer to thresholding filter
  vtkSmartPointer<vtkImageThreshold> _image_threshold;

  /// Pointer to the shift and scaling filter
  vtkSmartPointer<vtkImageShiftScale> _shift_scale_filter;

  /// Pointer to the magnitude filter
  vtkSmartPointer<vtkImageMagnitude> _magnitude_filter;
#endif

private:

  /**
   * Initializes image meta data such as image and voxel sizes
   */
  void initImageData();

  /**
   * Read an image(s)
   * @tparam T The type of vtk reader to utilize (e.g., vtkPNGReader)
   */
  template <typename T> void readImages();

  /**
   * Create the list of files to build the image data from
   */
  void getFiles();

  /// File base name
  FileName _file_base;

  /// File extension
  MooseEnum _file_type;

  /// Range of image files to open
  std::vector<unsigned int> _file_range;

  /// Origin of image
  std::vector<Real> _origin;

  /// Pixel dimension of image
  std::vector<int> _dims;

  /// Physical dimensions of image
  std::vector<Real> _physical_dims;

  /// Physical pixel size
  std::vector<double> _voxel;

  /// Component to extract
  unsigned int _component;

};

template <typename T>
void
ImageFunction::readImages()
{
#ifdef LIBMESH_HAVE_VTK
  // Indicate that data read has started
  _console << "Reading image(s)..." << std::endl;

  // Extract the data
  _image = vtkSmartPointer<T>::New();
  _image->SetFileNames(_files);
  _image->Update();
  _data = _image->GetOutput();

  // Set the image dimensions member variable
  int * dims = _data->GetDimensions();
  _dims.push_back(dims[0]);
  _dims.push_back(dims[1]);
  _dims.push_back(dims[2]);

  // Set the voxel size
  _voxel.push_back((_physical_dims[0] - _origin[0])/_dims[0]);
  _voxel.push_back((_physical_dims[1] - _origin[1])/_dims[1]);
  _voxel.push_back((_physical_dims[2] - _origin[2])/_dims[2]);

  // Indicate data read is completed
  _console << "          ...image read finished" << std::endl;
#endif
}

#endif // IMAGEFUNCTION_H

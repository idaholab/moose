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

#ifndef IMAGESAMPLER_H
#define IMAGESAMPLER_H

// MOOSE includes
#include "FileRangeBuilder.h"
#include "ConsoleStream.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

// VTK includes
#ifdef LIBMESH_HAVE_VTK

// Some VTK header files have extra semi-colons in them, and clang
// loves to warn about it...
#include "libmesh/ignore_warnings.h"

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
#include "vtkImageFlip.h"

#include "libmesh/restore_warnings.h"

#endif

// Forward declarations
class ImageSampler;
class MooseMesh;

template <>
InputParameters validParams<ImageSampler>();

/**
 * A helper class for reading and sampling images using VTK.
 */
class ImageSampler : public FileRangeBuilder
{
public:
  /**
   * Constructor.
   *
   * Use this object as an interface, being sure to also add the parameters to the
   * child class.
   *
   * @see ImageFunction
   */
  ImageSampler(const InputParameters & parameters);

  /**
   * Return the pixel value for the given point
   * @param p The point at which to extract pixel data
   */
  virtual Real sample(const Point & p);

  /**
   * Perform initialization of image data
   */
  virtual void setupImageSampler(MooseMesh & mesh);

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

  /**
   * Perform image flipping
   *
   * Flip the image along the x, y, and/or z axis. If multiple flips occur, they happen in order.
   */
  void vtkFlip();

private:
#ifdef LIBMESH_HAVE_VTK

  /// List of file names to extract data
  vtkSmartPointer<vtkStringArray> _files;

  /// Complete image data
  vtkImageData * _data;

  /// VTK-6 seems to work better in terms of "algorithm outputs" rather than vtkImageData pointers...
  vtkAlgorithmOutput * _algorithm;

  /// Complete image data
  vtkSmartPointer<vtkImageReader2> _image;

  /// Pointer to thresholding filter
  vtkSmartPointer<vtkImageThreshold> _image_threshold;

  /// Pointer to the shift and scaling filter
  vtkSmartPointer<vtkImageShiftScale> _shift_scale_filter;

  /// Pointer to the magnitude filter
  vtkSmartPointer<vtkImageMagnitude> _magnitude_filter;

  /// Pointers to image flipping filter.  May be used for x, y, or z.
  vtkSmartPointer<vtkImageFlip> _flip_filter;
#endif

/**
 * Helper method for flipping image
 * @param axis Flag for determing the flip axis: "x=0", "y=1", "z=2"
 * @return A smart pointer the flipping filter
 */
#ifdef LIBMESH_HAVE_VTK
  vtkSmartPointer<vtkImageFlip> imageFlip(const int & axis);
#endif

  /// Origin of image
  Point _origin;

  /// Pixel dimension of image
  std::vector<int> _dims;

  /// Physical dimensions of image
  Point _physical_dims;

  /// Physical pixel size
  std::vector<double> _voxel;

/// Component to extract
#ifdef LIBMESH_HAVE_VTK
  unsigned int _component;
#endif

  /// Bounding box for testing points
  MeshTools::BoundingBox _bounding_box;

  /// Parameters for interface
  const InputParameters & _is_pars;

  /// Create a console stream object for this helper class
  ConsoleStream _is_console;
};

#endif // IMAGESAMPLER_H

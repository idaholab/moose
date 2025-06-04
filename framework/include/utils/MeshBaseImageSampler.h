//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FileRangeBuilder.h"
#include "ConsoleStream.h"

#include "libmesh/bounding_box.h"
#include "libmesh/mesh_base.h"

// VTK includes
#ifdef LIBMESH_HAVE_VTK

// Some VTK header files have extra semi-colons in them, and clang
// loves to warn about it...
#include "libmesh/ignore_warnings.h"

// Include to define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
#include "VTKNlohmannUndefHack.h"
#if MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS && !defined(MOOSE_VTK_NLOHMANN_INCLUDED)
#undef INCLUDE_NLOHMANN_JSON_FWD_HPP_
#define MOOSE_VTK_NLOHMANN_INCLUDED
#endif

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

// If VTK is built without an external nlohmann, then it assumes it
// will never be compiled against another nlohmann, and it defines an
// nlohmann macro to point to its vtknlohmann copy.  In MOOSE their
// assumption is wrong.
#undef nlohmann

#include "libmesh/restore_warnings.h"

#endif

/**
 * A helper class for reading and sampling images using VTK, but with a Meshbase object
 */
class MeshBaseImageSampler : public FileRangeBuilder
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
  static InputParameters validParams();

  MeshBaseImageSampler(const InputParameters & parameters);

  /**
   * Return the pixel value for the given point
   * @param p The point at which to extract pixel data
   */
  virtual libMesh::Real sample(const libMesh::Point & p);

  /**
   * Perform initialization of image data
   */
  virtual void setupImageSampler(libMesh::MeshBase & mesh);

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
  libMesh::Point _origin;

  /// Pixel dimension of image
  std::vector<int> _dims;

  /// Physical dimensions of image
  libMesh::Point _physical_dims;

  /// Physical pixel size
  std::vector<double> _voxel;

/// Component to extract
#ifdef LIBMESH_HAVE_VTK
  unsigned int _component;
#endif

  /// Bounding box for testing points
  libMesh::BoundingBox _bounding_box;

  /// Parameters for interface
  const InputParameters & _is_pars;

  /// Create a console stream object for this helper class
  ConsoleStream _is_console;
};

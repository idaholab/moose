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
#include <array>

// VTK includes
#ifdef LIBMESH_HAVE_VTK

// Some VTK header files have extra semi-colons in them, and clang
// loves to warn about it...
#include "libmesh/ignore_warnings.h"

// If VTK is built without an external nlohmann, then it assumes it
// will never be compiled against another nlohmann, and it includes
// its own copy but modified with macro tricks.  We have probably
// already included nlohmann headers, which we didn't tamper with
// because OF COURSE NOT, but now we need to take care not to let the
// include guards prevent them from including their copy with their
// different namespace.
#ifndef MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
#define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS 0
// Detect if VTK built with external nlohmann
#ifdef __has_include
#if __has_include("vtk_nlohmannjson.h")
#include "vtk_nlohmannjson.h"
#if !VTK_MODULE_USE_EXTERNAL_vtknlohmannjson
#undef MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS
#define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS 1
#endif // !VTK_MODULE_USE_EXTERNAL_vtknlohmannjson
#endif // __has_include("vtk_nlohmannjson.h")
#else  // __has_include
#error "Could not auto-detect whether VTK built with external nlohmann json. \
Define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS=1 if built with vendored nlohmann \
, otherwise define MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS=0"
#endif // __has_include
#endif // MOOSE_VTK_UNDEF_NLOHMANNJSON_HEADER_GUARDS

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

class MooseMesh;

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
  static InputParameters validParams();

  ImageSampler(const InputParameters & parameters);

  /**
   * Return the pixel value for the given point
   * @param p The point at which to extract pixel data
   */
  virtual libMesh::Real sample(const libMesh::Point & p) const;

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

  /// image flip
  std::array<bool, 3> _flip;
};

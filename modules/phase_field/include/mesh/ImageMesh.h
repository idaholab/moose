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

#ifndef IMAGEMESH_H
#define IMAGEMESH_H

#include "GeneratedMesh.h"

class ImageMesh;

template<>
InputParameters validParams<ImageMesh>();

/**
 * A 2D GeneratedMesh where xmin, xmax, etc. are determined from an input image file.
 */
class ImageMesh : public GeneratedMesh
{
public:
  ImageMesh(const std::string & name, InputParameters parameters);
  ImageMesh(const ImageMesh & other_mesh);
  virtual ~ImageMesh();

  virtual MooseMesh & clone() const;

  virtual void buildMesh();

protected:
  /**
   * buildMesh() calls this helper function to build 2D ImageMeshes.
   */
  void buildMesh2D();

  /**
   * buildMesh() calls this helper function to build 3D ImageMeshes from stacks of images.
   */
  void buildMesh3D();

  /**
   * True if the user provided the "file" param, see below for details.
   */
  bool _has_file;

  /**
   * The name of the image to extract Mesh parameters from.  The
   * current implementation processes the output of the 'file'
   * command, which typically exists on OSX and Linux.
   */
  std::string _file;

  /**
   * True if the user provided the "file_base" param, see below for details.
   */
  bool _has_file_base;

  /**
   * File base name used in conjunction with file_range
   */
  std::string _file_base;

  /**
   * True if the user provided the "file_range" param, see below for details.
   */
  bool _has_file_range;

  /**
   * Vector with at most two entries which defines the range of image files to open.
   */
  std::vector<unsigned int> _file_range;

  /**
   * True if the user provided the "file_suffix" param, see below for details.
   */
  bool _has_file_suffix;

  /**
   * String representing the file suffix.  The filenames are assumed to be of the form:
   * file_base + (zero-padded number in file range) + . + file_suffix
   */
  std::string _file_suffix;

  /**
   * If true, forces the maximum width (height) of the mesh to be 1.0
   * while retaining the original aspect ratio of the image.
   */
  bool _scale_to_one;

  /**
   * A number <= 1.0 which determines the number of cells in the mesh
   * per pixel in each direction. Defaults to 1.0
   * Example:
   * Given:  Original image is 1843x1590 pixels
   *         _cells_per_pixel = 0.3
   * Result: Mesh has 552x477 elements
   */
  Real _cells_per_pixel;

  /**
   * Stores the list of filenames in the "stack" when building 3D
   * meshes from multiple images.
   */
  std::vector<std::string> _stack_filenames;

  /**
   * Process a single image with the 'file' command to find out the
   * number of pixels in the x and y directions.
   */
  void GetPixelInfo(std::string filename, int & xpixels, int & ypixels);
};

#endif /* IMAGEMESH_H */

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneratedMeshGenerator.h"
#include "FileRangeBuilder.h"
#include "libmesh/replicated_mesh.h"

/**
 * A 2D GeneratedMesh where xmin, xmax, etc. are determined from an input image file.
 */
class ImageMeshGenerator : public GeneratedMeshGenerator, public FileRangeBuilder
{
public:
  static InputParameters validParams();

  ImageMeshGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /**
   * generate() calls this helper function to build 2D ImageMeshes.
   */
  void buildMesh2D(const std::string & filename, MeshBase & mesh);

  /**
   * generate() calls this helper function to build 3D ImageMeshes from stacks of images.
   */
  void buildMesh3D(const std::vector<std::string> & filenames, MeshBase & mesh);

  /**
   * Process a single image with the 'file' command to find out the
   * number of pixels in the x and y directions.
   */
  void GetPixelInfo(std::string filename, int & xpixels, int & ypixels);

  /**
   * If true, forces the maximum width (height) of the mesh to be 1.0
   * while retaining the original aspect ratio of the image.
   */
  const bool _scale_to_one;

  /**
   * A number <= 1.0 which determines the number of cells in the mesh
   * per pixel in each direction. Defaults to 1.0
   * Example:
   * Given:  Original image is 1843x1590 pixels
   *         _cells_per_pixel = 0.3
   * Result: Mesh has 552x477 elements
   */
  const Real & _cells_per_pixel;
};

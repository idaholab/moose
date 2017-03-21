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
#include "FileRangeBuilder.h"

class ImageMesh;

template <>
InputParameters validParams<ImageMesh>();

/**
 * A 2D GeneratedMesh where xmin, xmax, etc. are determined from an input image file.
 */
class ImageMesh : public GeneratedMesh, public FileRangeBuilder
{
public:
  ImageMesh(const InputParameters & parameters);
  ImageMesh(const ImageMesh & other_mesh);

  virtual MooseMesh & clone() const override;

  virtual void buildMesh() override;

protected:
  /**
   * buildMesh() calls this helper function to build 2D ImageMeshes.
   */
  void buildMesh2D(const std::string & filename);

  /**
   * buildMesh() calls this helper function to build 3D ImageMeshes from stacks of images.
   */
  void buildMesh3D(const std::vector<std::string> & filenames);

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

#endif /* IMAGEMESH_H */

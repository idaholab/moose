/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
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
  void buildMesh2D(const std::string & filename);

  /**
   * buildMesh() calls this helper function to build 3D ImageMeshes from stacks of images.
   */
  void buildMesh3D(const std::vector<std::string> & filenames);

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
   * Process a single image with the 'file' command to find out the
   * number of pixels in the x and y directions.
   */
  void GetPixelInfo(std::string filename, int & xpixels, int & ypixels);
};

#endif /* IMAGEMESH_H */

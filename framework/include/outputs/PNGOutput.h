//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Forward declarations
class PNGOutput;

template <>
InputParameters validParams<PNGOutput>();

// pnglib include
#include <png.h>
// MOOSE includes
#include "Output.h"

class PNGOutput : public Output
{
public:
  // Basic constructor.  Takes parameters passed in to create a PNGOutput object.
  PNGOutput(const InputParameters & parameters);

protected:
  // Method for assigning color values to the PNG
  void setRGB(png_byte * rgb, Real selection);

  // Function to create the mesh_function
  void makeMeshFunc();

  // Function to populate values to the variables used for scaling
  void calculateRescalingValues();

  // Function that creates the PNG
  void makePNG();

  // Called to run the functions in this class.
  virtual void output(const ExecFlagType & /*type*/);

  // Variable to determine the size, or resolution, of the image.
  Real resolution;

  // Name of file to which the PNG will be written.
  std::string PNGFile;

  // Specific test step to create the image for.
  Real testStepToPNG;

  // Way to specify color vs grayscale image creation.
  bool inColor;

  // Way to track the number test step number and the associated png, if created.
  Real testStep = 0;

  /// Pointer the libMesh::MeshFunction object that the read data is stored
  std::unique_ptr<MeshFunction> _mesh_function;

  // The boundaries of the image.
  BoundingBox box;

  // Values used for rescaling the image.
  Real scalingMin;
  Real scalingMax;
  Real shiftValue;

  // Value of the colorrs that are outside of the libmesh bounds.
  Real outBoundsControlColor;
};

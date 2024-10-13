//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once

#include "MooseConfig.h"
#ifdef MOOSE_HAVE_LIBPNG

// pnglib includes
#include <png.h>
// MOOSE includes
#include "FileOutput.h"
#include "MooseEnum.h"
// libmesh includes
#include "libmesh/mesh_function.h"
#include "libmesh/bounding_box.h"

class PNGOutput : public FileOutput
{
public:
  /// Basic constructor.  Takes parameters passed in to create a PNGOutput object.
  static InputParameters validParams();

  PNGOutput(const InputParameters & parameters);

protected:
  /// Method for assigning color values to the PNG
  void setRGB(png_byte * rgb, Real selection);

  /// Function to create the mesh_function
  void makeMeshFunc();

  /// Function to populate values to the variables used for scaling
  void calculateRescalingValues();

  /// Function for applying scaling to given values.
  Real applyScale(Real value_to_scale);

  /// Function for reversing the applyScale function.
  Real reverseScale(Real value_to_unscale);

  /// Function that creates the PNG
  void makePNG();

  /// Called to run the functions in this class.
  virtual void output();

  /// Variable to determine the size, or resolution, of the image.
  const unsigned int _resolution;

  /// Way to specify color vs grayscale image creation.
  const MooseEnum _color;

  /// Indicates whether to make the background transparent.
  const bool _transparent_background;

  /// Controls transparency level for the general image.
  const Real _transparency;

  /// Pointer to the libMesh::MeshFunction object in which the read data is stored.
  std::unique_ptr<libMesh::MeshFunction> _mesh_function;

  /// The boundaries of the image.
  BoundingBox _box;

  /// What nonlinear system the variable is in. If the value is invalid, then the variable is in the
  /// auxiliary system
  unsigned int _nl_sys_num;

  /// The name of the variable to use to create the png.
  VariableName _variable;

  /// Variables that store the max and min of the values in the variable used.
  /// Used for creating the scaling values.
  Real _max;
  Real _min;

  /// Values used for rescaling the image.
  Real _scaling_min;
  Real _scaling_max;
  Real _shift_value;

  /// Value of the colors that are outside of the libmesh bounds.
  Real _out_bounds_shade;
};

#endif

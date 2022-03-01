//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Function.h"
#include "ImageSampler.h"

/**
 * A function for extracting data from an image or stack of images
 */
class ImageFunction : public ImageSampler, public Function
{
public:
  /**
   * Class constructor
   * @param parameters The parameters object holding data for the class to use.
   */
  static InputParameters validParams();

  ImageFunction(const InputParameters & parameters);

  /**
   * Class destructor
   */
  virtual ~ImageFunction();

  /**
   * Initialize the ImageSampler
   */
  virtual void initialSetup() override;

  using Function::value;
  /**
   * Return the pixel value for the given point
   * @param t Time (unused)
   * @param p The point at which to extract pixel data
   */
  virtual Real value(Real t, const Point & p) const override;
};

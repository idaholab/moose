//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef IMAGESUBDOMAIN_H
#define IMAGESUBDOMAIN_H

// MOOSE includes
#include "MeshModifier.h"
#include "ImageSampler.h"

// Forward declarations
class ImageSubdomain;

template <>
InputParameters validParams<ImageSubdomain>();

/**
 * MeshModifier for defining a Subdomains based on Image data.
 */
class ImageSubdomain : public MeshModifier, public ImageSampler
{
public:
  /**
   * Class constructor
   * @param parameters The input parameters
   */
  ImageSubdomain(const InputParameters & parameters);

protected:
  virtual void modify() override;
};

#endif // IMAGESUBDOMAIN_H

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

#ifndef IMAGESUBDOMAIN_H
#define IMAGESUBDOMAIN_H

// MOOSE includes
#include "MeshModifier.h"
#include "ImageSampler.h"

// libmesh includes
#include "libmesh/mesh_tools.h"

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

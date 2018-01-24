//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)
#include <cmath>

// MOOSE includes
#include "ImageSubdomain.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ImageSubdomain>()
{
  InputParameters params = validParams<MeshModifier>();
  params.addClassDescription("Samples an image at the coordinates of each element centroid using "
                             "the resulting value as each element's subdomain ID");
  params += validParams<ImageSampler>();
  return params;
}

ImageSubdomain::ImageSubdomain(const InputParameters & parameters)
  : MeshModifier(parameters), ImageSampler(parameters)
{
}

void
ImageSubdomain::modify()
{
  // Check that we have access to the mesh
  if (!_mesh_ptr)
    mooseError("_mesh_ptr must be initialized before calling SubdomainBoundingBox::modify()");

  // Initialize the ImageSampler
  setupImageSampler(*_mesh_ptr);

  // Reference the the libMesh::MeshBase
  MeshBase & mesh = _mesh_ptr->getMesh();

  // Loop over the elements and sample the image at the element centroid and use the value for the
  // subdomain id
  for (auto & elem : mesh.active_element_ptr_range())
  {
    SubdomainID id = static_cast<SubdomainID>(round(sample(elem->centroid())));
    elem->subdomain_id() = id;
  }
}

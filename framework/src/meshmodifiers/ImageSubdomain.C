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

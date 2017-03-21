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

// STL includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

// MOOSE includes
#include "ImageSubdomain.h"
#include "MooseMesh.h"

template <>
InputParameters
validParams<ImageSubdomain>()
{
  InputParameters params = validParams<MeshModifier>();
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
  for (MeshBase::element_iterator el = mesh.active_elements_begin();
       el != mesh.active_elements_end();
       ++el)
  {
    SubdomainID id = static_cast<SubdomainID>(round(sample((*el)->centroid())));
    (*el)->subdomain_id() = id;
  }
}

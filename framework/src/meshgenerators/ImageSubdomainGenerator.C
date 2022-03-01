//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ImageSubdomainGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"

// provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)
#include <cmath>

registerMooseObject("MooseApp", ImageSubdomainGenerator);

InputParameters
ImageSubdomainGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addClassDescription("Samples an image at the coordinates of each element centroid, using "
                             "the resulting pixel color value as each element's subdomain ID");
  params += MeshBaseImageSampler::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");

  return params;
}

ImageSubdomainGenerator::ImageSubdomainGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), MeshBaseImageSampler(parameters), _input(getMesh("input"))
{
}

std::unique_ptr<MeshBase>
ImageSubdomainGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // Initialize the ImageSampler
  setupImageSampler(*mesh);

  // Loop over the elements and sample the image at the element centroid and use the value for the
  // subdomain id
  for (auto & elem : mesh->active_element_ptr_range())
  {
    subdomain_id_type id = static_cast<subdomain_id_type>(round(sample(elem->vertex_average())));
    elem->subdomain_id() = id;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}

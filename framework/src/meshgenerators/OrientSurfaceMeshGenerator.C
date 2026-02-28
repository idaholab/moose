//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrientSurfaceMeshGenerator.h"
#include "Parser.h"
#include "InputParameters.h"
#include "CastUniquePointer.h"

#include "libmesh/fe_base.h"
#include "libmesh/mesh_generation.h"
#include "libmesh/mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/quadrature_gauss.h"
#include "libmesh/point_locator_base.h"
#include "libmesh/distributed_mesh.h"
#include "libmesh/elem.h"

#include <typeinfo>

registerMooseObject("MooseApp", OrientSurfaceMeshGenerator);

InputParameters
OrientSurfaceMeshGenerator::validParams()
{
  InputParameters params = SubdomainsGeneratorBase::validParams();

  // Which elements to apply the change on
  params.setDocString(
      "included_subdomains",
      "Subdomain names or ids for the elements that may have their normal modified.");

  // How to set the normal
  params.renameParam("normal",
                     "normal_to_align_with",
                     "Direction vector that element normals should be pointing in the same "
                     "direction as (dot product > 0)");
  params.suppressParameter<Real>("normal_tol");
  params.suppressParameter<Real>("flipped_normal_tol");
  params.suppressParameter<bool>("fixed_normal");
  params.suppressParameter<bool>("allow_normal_flips");

  // Flooding / spreading algorithm parameters are not used
  // as the flooding is not used for now
  params.suppressParameter<std::vector<Real>>("max_paint_size_centroids");
  params.suppressParameter<bool>("check_painted_neighbor_normals");

  // This mesh generator does not modify the subdomains
  params.suppressParameter<std::vector<SubdomainName>>("new_subdomain");
  // We are changing the normals, not using them to paint
  params.addPrivateParam<bool>("_using_normal", false);

  params.addClassDescription("Change the orientation of (part of) the surface mesh.");
  return params;
}

OrientSurfaceMeshGenerator::OrientSurfaceMeshGenerator(const InputParameters & parameters)
  : SubdomainsGeneratorBase(parameters)
{
}

std::unique_ptr<MeshBase>
OrientSurfaceMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  setup(*mesh);

  unsigned int num_flipped = 0;
  auto binfo = mesh->get_boundary_info();

  // We'll need to loop over all of the elements to adjust normals
  for (auto & elem : mesh->element_ptr_range())
  {
    // Nothing to do with edges
    if (elem->dim() < 2)
      continue;
    // Nothing to do with 3D elements
    if (elem->dim() > 2)
      continue;

    // Check if element should be used to paint from
    if (_included_subdomain_ids.size() &&
        std::find(_included_subdomain_ids.begin(),
                  _included_subdomain_ids.end(),
                  elem->subdomain_id()) == _included_subdomain_ids.end())
      continue;

    // Compute the normal
    const auto normal = get2DElemNormal(elem);

    if (normal * _normal < 0)
    {
      elem->flip(&binfo);
      num_flipped++;
    }
  }

  if (num_flipped)
    _console << "Flipped the orientation of " << num_flipped << " surface elements." << std::endl;

  return dynamic_pointer_cast<MeshBase>(mesh);
}

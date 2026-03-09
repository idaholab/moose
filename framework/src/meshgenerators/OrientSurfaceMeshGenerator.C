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
  InputParameters params = SurfaceMeshGeneratorBase::validParams();

  // NOTE: libmesh tetrahedralization code actually has a clever heuristic
  // to re-orient elements starting from the known orientation of the "bottom-most"
  // surface element. We could import that logic here, once we identify and separate connected
  // (contiguous) components of the mesh.

  // Which elements to apply the change on
  params.setDocString(
      "included_subdomains",
      "Subdomain names or ids for the elements that may have their normal modified.");

  // How to set the normal
  params.renameParam("normal",
                     "normal_to_align_with",
                     "Direction vector that element normals should be pointing in the same "
                     "direction as (dot product > 0)");
  params.addParam<std::vector<dof_id_type>>(
      "element_ids_to_flood_from",
      "IDs of elements to start flooding and changing the subdomains from");

  // This mesh generator does not modify the subdomains
  params.suppressParameter<std::vector<SubdomainName>>("new_subdomain");
  // This mesh generator is specifically intended to flip normals
  params.set<bool>("flip_inverted_normals") = true;
  params.suppressParameter<bool>("flip_inverted_normals");
  params.set<bool>("consider_flipped_normals") = true;
  params.set<bool>("_using_normal") = true;

  params.addClassDescription("Change the orientation of (part of) the surface mesh.");
  return params;
}

OrientSurfaceMeshGenerator::OrientSurfaceMeshGenerator(const InputParameters & parameters)
  : SurfaceMeshGeneratorBase(parameters)
{
  if (!isParamValid("element_ids_to_flood_from") && !isParamSetByUser("normal_to_align_with"))
    paramError("normal_to_align_with",
               "Either a 'normal_to_align_with' or 'element_ids_to_flood_from' must be specified "
               "to select the behavior of this mesh generator.");
}

std::unique_ptr<MeshBase>
OrientSurfaceMeshGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);
  setup(*mesh);

  unsigned int num_flipped = 0;
  auto & binfo = mesh->get_boundary_info();
  bool normal_is_input = isParamSetByUser("normal_to_align_with");

  if (!isParamValid("element_ids_to_flood_from"))
    // We'll need to loop over all of the elements to adjust normals with the fixed normal option
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
  else
    // We'll flood and re-adjust orientations starting from a few given elements
    // NOTE: user's responsibility to make sure these surface elements' orientations are consistent
    for (const auto eid : getParam<std::vector<dof_id_type>>("element_ids_to_flood_from"))
    {
      auto elem = mesh->elem_ptr(eid);

      // Nothing to do with edges
      if (elem->dim() < 2)
        continue;
      // Nothing to do with 3D elements
      if (elem->dim() > 2)
        continue;

      // Compute the normal
      const auto normal = normal_is_input ? _normal : get2DElemNormal(elem);

      flood(elem, normal, *elem, elem->subdomain_id(), *mesh);
    }

  if (num_flipped)
    _console << "Flipped the orientation of " << num_flipped << " surface elements." << std::endl;

  return dynamic_pointer_cast<MeshBase>(mesh);
}

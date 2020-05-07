/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                 Rattlesnake                   */
/*                                               */
/*    (c) 2017 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     Under Contract No. DE-AC07-05ID14517      */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#include "GeneratedIDMeshGenerator.h"

#include "libmesh/elem.h"

registerMooseObject("MooseApp", GeneratedIDMeshGenerator);

defineLegacyParams(GeneratedIDMeshGenerator);

InputParameters
GeneratedIDMeshGenerator::validParams()
{
  InputParameters params = GeneratedMeshGenerator::validParams();
  params.addParam<std::vector<unsigned int>>("subdomain_id", "Subdomain IDs, default to all zero");
  params.addParam<std::vector<unsigned int>>("material_id", "Material IDs, default to all invalid");
  params.addParam<std::vector<unsigned int>>("depletion_id",
                                             "Depletion zone IDs, default to all invalid");
  params.addParam<std::vector<unsigned int>>("equivalence_id",
                                             "Equivalence zone IDs, default to all invalid");
  params.addClassDescription(
      "Construct a mesh with specified IDs (material, depletion and equivalence).");
  return params;
}

GeneratedIDMeshGenerator::GeneratedIDMeshGenerator(const InputParameters & params)
  : GeneratedMeshGenerator(params),
    _has_subdomain_id(params.isParamValid("subdomain_id")),
    _has_material_id(params.isParamValid("material_id")),
    _has_depletion_id(params.isParamValid("depletion_id")),
    _has_equivalence_id(params.isParamValid("equivalence_id"))
{
}

std::unique_ptr<MeshBase>
GeneratedIDMeshGenerator::generate()
{
  auto mesh = GeneratedMeshGenerator::generate();

  std::vector<unsigned int> bids; // subdomain IDs
  std::vector<unsigned int> mids; // material IDs
  std::vector<unsigned int> dids; // depletion IDs
  std::vector<unsigned int> eids; // equivalence IDs

  unsigned int ngrid;
  if (_dim == 1)
    ngrid = _nx;
  else if (_dim == 2)
    ngrid = _nx * _ny;
  else
    ngrid = _nx * _ny * _nz;

  if (_has_subdomain_id)
  {
    bids = getParam<std::vector<unsigned int>>("subdomain_id");
    if (bids.size() != ngrid)
      mooseError("Size of subdomain_id is not consistent");
  }
  else
    bids.assign(ngrid, 0);

  unsigned int material_id;
  if (_has_material_id)
  {
    material_id = mesh->add_elem_integer("material_id");
    mids = getParam<std::vector<unsigned int>>("material_id");
    if (mids.size() != ngrid)
      mooseError("Size of material_id is not consistent");
  }

  unsigned int depletion_id;
  if (_has_depletion_id)
  {
    depletion_id = mesh->add_elem_integer("depletion_id");
    dids = getParam<std::vector<unsigned int>>("depletion_id");
    if (dids.size() != ngrid)
      mooseError("Size of depletion_id is not consistent");
  }

  unsigned int equivalence_id;
  if (_has_equivalence_id)
  {
    equivalence_id = mesh->add_elem_integer("equivalence_id");
    eids = getParam<std::vector<unsigned int>>("equivalence_id");
    if (eids.size() != ngrid)
      mooseError("Size of equivalence_id is not consistent");
  }

  for (auto & elem : mesh->active_element_ptr_range())
  {
    const Point p = elem->centroid();
    unsigned int ix = std::floor((p(0) - _xmin) / (_xmax - _xmin) * _nx);
    unsigned int iy = std::floor((p(1) - _ymin) / (_ymax - _ymin) * _ny);
    unsigned int iz = std::floor((p(2) - _zmin) / (_zmax - _zmin) * _nz);
    unsigned int i = iz * _nx * _ny + iy * _nx + ix;
    elem->subdomain_id() = bids[i];
    if (_has_material_id)
      elem->set_extra_integer(material_id, mids[i]);
    if (_has_depletion_id)
      elem->set_extra_integer(depletion_id, dids[i]);
    if (_has_equivalence_id)
      elem->set_extra_integer(equivalence_id, eids[i]);
  }

  return mesh;
}

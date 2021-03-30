//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "FVPorosityPerSubdomainMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "SinglePhaseFluidProperties.h"

registerMooseObject("NavierStokesApp", FVPorosityPerSubdomainMaterial);

InputParameters
FVPorosityPerSubdomainMaterial::validParams()
{
  auto params = Material::validParams();
  params.addClassDescription("Computes porosity on a per-subdomain basis");
  params.addRequiredParam<std::map<std::string, Real>>("subdomain_to_porosity",
                                                       "Map from subdomain to porosity value");
  return params;
}

FVPorosityPerSubdomainMaterial::FVPorosityPerSubdomainMaterial(const InputParameters & params)
  : Material(params), _epsilon(declareProperty<Real>(NS::porosity))
{
  for (const auto & map_pr : getParam<std::map<std::string, Real>>("subdomain_to_porosity"))
    _sub_id_to_epsilon.emplace(std::make_pair(_mesh.getSubdomainID(map_pr.first), map_pr.second));
}

void
FVPorosityPerSubdomainMaterial::computeQpProperties()
{
  if (!_bnd)
  {
    mooseAssert(_current_elem,
                "We should be on a block which means we should definitely have a current element");
    auto it = _sub_id_to_epsilon.find(_current_elem->subdomain_id());
    mooseAssert(it != _sub_id_to_epsilon.end(),
                "Block restriction must match the subdomain names passed in the "
                "subdomain_to_porosity parameter");
    _epsilon[_qp] = it->second;

    return;
  }

  mooseAssert(_face_info,
              "We must have set a face info object in order for the FVPorosityPerSubdomainMaterial "
              "class to work on faces");

  if (_current_elem)
  {
    mooseAssert(_current_elem->build_side_ptr(_current_side)->centroid() ==
                    _face_info->faceCentroid(),
                "Making sure we're in the right place");
    auto it = _sub_id_to_epsilon.find(_current_elem->subdomain_id());
    if (it == _sub_id_to_epsilon.end())
    {
      // We may be a ghosted material
      const bool current_elem_is_fi_elem = _current_elem == &_face_info->elem();
      const Elem * const other_elem_to_try =
          current_elem_is_fi_elem ? _face_info->neighborPtr() : &_face_info->elem();
      mooseAssert(other_elem_to_try, "This should be non-null");
      it = _sub_id_to_epsilon.find(other_elem_to_try->subdomain_id());
      mooseAssert(it != _sub_id_to_epsilon.end(),
                  "Block restriction must match the subdomain names passed in the "
                  "subdomain_to_porosity parameter");
    }
    _epsilon[_qp] = it->second;
    return;
  }

  // We must be off the domain
  auto it = _sub_id_to_epsilon.find(_face_info->elem().subdomain_id());
  mooseAssert(it != _sub_id_to_epsilon.end(),
              "Block restriction must match the subdomain names passed in the "
              "subdomain_to_porosity parameter");
  _epsilon[_qp] = it->second;
}

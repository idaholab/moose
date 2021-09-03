//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "PolygonMeshGeneratorBase.h"
#include "MooseEnum.h"
#include "MeshMetaDataInterface.h"

class SimpleHexagonGenerator : public PolygonMeshGeneratorBase
{
public:
  static InputParameters validParams();

  SimpleHexagonGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  const Real _hexagon_size;
  const enum class HexagonStyle { apothem, radius } _hexagon_size_style;
  Real _pitch;
  const bool _block_id_valid;
  const subdomain_id_type _block_id;
  const SubdomainName _block_name;
  const bool _boundary_id_valid;
  const boundary_id_type _external_boundary_id;
  const std::string _external_boundary_name;

  Real & _pitch_meta;
  const unsigned int _background_intervals_meta;
  const unsigned int _node_id_background_meta;
  const Real _max_radius_meta;
  const std::vector<unsigned int> _num_sectors_per_side_meta;
};

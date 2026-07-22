//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "libmesh/quadrature_gauss.h"
#include "FunctionParserUtils.h"
#include "libmesh/fe.h"

class SubdomainInterceptedGenerator : public MeshGenerator, public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();
  SubdomainInterceptedGenerator(const InputParameters & parameters);

  std::unique_ptr<libMesh::MeshBase> generate() override;

protected:
  std::unique_ptr<libMesh::MeshBase> & _input;

  /// Parsed function
  SymFunctionPtr _parsed_function;

protected:
  /// IDs for subdomain classification
  SubdomainID _subdomain_id_inside;
  SubdomainID _subdomain_id_outside;

  /// Threshold value for classification
  Real _threshold;

  /// Lambda parameter for false intersection classification
  Real _lambda;

  /// Outer boundary handling flag
  bool _outer_boundary;

  /// for multi-geometry handling
  bool _multi_geo;
  bool _keep_inside_as_inside;
  bool _keep_outside_as_outside;

  // Only modify outside elements (for multi-geometry handling)
  bool _modify_outside_only;
  bool _modify_inside_only;

  /// @brief  Quadrature order used for active‑area estimation
  int _qrule_order;

  usingFunctionParserUtilsMembers(false);

  /// @brief Convert integer to Order enum
  static Order intToOrder(int value);
};

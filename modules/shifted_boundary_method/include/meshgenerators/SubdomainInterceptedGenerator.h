//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMSubdomainGeneratorBase.h"
#include "FunctionParserUtils.h"

/**
 * Classifies mesh elements into inside, outside, and optionally intercepted subdomains
 * based on an analytical signed distance function expression.
 */
class SubdomainInterceptedGenerator : public SBMSubdomainGeneratorBase,
                                      public FunctionParserUtils<false>
{
public:
  static InputParameters validParams();
  SubdomainInterceptedGenerator(const InputParameters & parameters);

  std::unique_ptr<libMesh::MeshBase> generate() override;

protected:
  /// Parsed function
  SymFunctionPtr _parsed_function;

protected:
  /// Subdomain ID assigned to elements categorized inside the domain
  SubdomainID _subdomain_id_inside;
  /// Subdomain ID assigned to elements categorized outside the domain
  SubdomainID _subdomain_id_outside;

  /// Level-set threshold value defining the interface boundary
  Real _threshold;

  /// Whether the inside domain corresponds to the negative signed distance region (< threshold)
  bool _is_domain_inside_surface;

  /// Whether multi-geometry handling is enabled to skip previously assigned elements
  bool _multi_geo;
  /// If multi_geo is true, whether to keep elements already matching subdomain_id_inside
  bool _keep_inside_as_inside;
  /// If multi_geo is true, whether to keep elements already matching subdomain_id_outside
  bool _keep_outside_as_outside;

  /// If true, only modify outside elements; inside elements retain their existing subdomain IDs
  bool _modify_outside_only;
  /// If true, only modify inside elements; outside elements retain their existing subdomain IDs
  bool _modify_inside_only;

  usingFunctionParserUtilsMembers(false);
};

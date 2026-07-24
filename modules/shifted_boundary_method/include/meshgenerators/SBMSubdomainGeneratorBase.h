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
#include "libmesh/enum_order.h"

/**
 * Common parameters and members for shifted boundary subdomain mesh generators that classify volume
 * elements against a geometry using quadrature-based active-area estimation.
 */
class SBMSubdomainGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();
  SBMSubdomainGeneratorBase(const InputParameters & parameters);

protected:
  /// The input mesh we want to modify
  std::unique_ptr<libMesh::MeshBase> & _input;

  /// Quadrature order used to estimate the active area
  const Order _qrule_order;
};

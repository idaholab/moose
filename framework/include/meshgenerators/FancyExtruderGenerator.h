//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FANCYEXTRUDERGENERATOR_H
#define FANCYEXTRUDERGENERATOR_H

#include "MeshGenerator.h"

// Forward declarations
class FancyExtruderGenerator;

template <>
InputParameters validParams<FancyExtruderGenerator>();

/**
 * Generates a mesh by reading it from an file.
 */
class FancyExtruderGenerator : public MeshGenerator
{
public:
  FancyExtruderGenerator(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() override;

protected:
  /// Height of each elevation
  const std::vector<Real> & _heights;

  /// Number of layers in each elevation
  const std::vector<Real> & _num_layers;

  /// Subdomains to swap out for each elevation
  const std::vector<std::vector<Real>> & _subdomain_swaps;
};

#endif // FANCYEXTRUDERGENERATOR_H

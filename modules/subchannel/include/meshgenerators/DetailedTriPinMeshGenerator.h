#pragma once

#include "DetailedPinMeshGeneratorBase.h"

/**
 * Mesh generator for fuel pins in a quadrilateral lattice
 */
class DetailedTriPinMeshGenerator : public DetailedPinMeshGeneratorBase
{
public:
  DetailedTriPinMeshGenerator(const InputParameters & parameters);

  virtual std::unique_ptr<MeshBase> generate() override;

protected:
  /// Mesh that comes from another generator
  std::unique_ptr<MeshBase> & _input;
  /// Number if rings in the fuel assembly
  const unsigned int & _n_rings;

public:
  static InputParameters validParams();
};

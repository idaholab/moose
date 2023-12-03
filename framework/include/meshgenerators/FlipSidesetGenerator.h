#pragma once

#include "MeshGenerator.h"

/**
 * MeshGenerator for flipping sideset
 */
class FlipSidesetGenerator : public MeshGenerator
{
public:
  static InputParameters validParams();

  FlipSidesetGenerator(const InputParameters & parameters);

protected:
  std::unique_ptr<MeshBase> generate() override;

private:
  ///Input mesh the operation will be applied to
  std::unique_ptr<MeshBase> & _input;

  ///Name of the sideset to flip
  const BoundaryName _sideset_name;
};

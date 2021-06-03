#pragma once

#include "BetterQuadSubChannelMesh.h"

class BetterDetailedSubChannelMesh : public BetterQuadSubChannelMesh
{
public:
  BetterDetailedSubChannelMesh(const InputParameters & parameters);
  virtual void buildMesh() override;

public:
  static InputParameters validParams();
};

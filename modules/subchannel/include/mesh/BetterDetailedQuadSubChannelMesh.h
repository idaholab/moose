#pragma once

#include "BetterQuadSubChannelMesh.h"

class BetterDetailedQuadSubChannelMesh : public BetterQuadSubChannelMesh
{
public:
  BetterDetailedQuadSubChannelMesh(const InputParameters & parameters);
  virtual void buildMesh() override;

public:
  static InputParameters validParams();
};

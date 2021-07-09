#pragma once

#include "QuadSubChannelMesh.h"

class DetailedQuadSubChannelMesh : public QuadSubChannelMesh
{
public:
  DetailedQuadSubChannelMesh(const InputParameters & parameters);
  virtual void buildMesh() override;

public:
  static InputParameters validParams();
};

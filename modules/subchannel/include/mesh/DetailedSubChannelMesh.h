#pragma once

#include "QuadSubChannelMesh.h"

class DetailedSubChannelMesh : public QuadSubChannelMesh
{
public:
  DetailedSubChannelMesh(const InputParameters & parameters);
  virtual void buildMesh() override;

public:
  static InputParameters validParams();
};

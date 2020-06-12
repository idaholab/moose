#pragma once

#include "SubChannelMesh.h"

class DetailedSubChannelMesh : public SubChannelMesh
{
public:
  DetailedSubChannelMesh(const InputParameters & parameters);

  virtual void buildMesh() override;

public:
  static InputParameters validParams();
};

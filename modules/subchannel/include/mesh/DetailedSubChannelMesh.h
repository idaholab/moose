#pragma once

#include "SubChannelMesh.h"

class DetailedSubChannelMesh;

template <>
InputParameters validParams<DetailedSubChannelMesh>();

class DetailedSubChannelMesh : public SubChannelMesh
{
public:
  DetailedSubChannelMesh(const InputParameters & parameters);

  virtual void buildMesh() override;
};

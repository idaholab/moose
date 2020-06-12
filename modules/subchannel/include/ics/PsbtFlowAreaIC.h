#pragma once

#include "PsbtIC.h"
#include "SolutionHandle.h"
#include "SubChannelMesh.h"

class PsbtFlowAreaIC : public PsbtIC
{
public:
  PsbtFlowAreaIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;

public:
  static InputParameters validParams();
};

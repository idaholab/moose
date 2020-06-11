#pragma once

#include "PsbtIC.h"
#include "SolutionHandle.h"
#include "SubChannelMesh.h"

class PsbtFlowAreaIC;

template <>
InputParameters validParams<PsbtFlowAreaIC>();

class PsbtFlowAreaIC : public PsbtIC
{
public:
  PsbtFlowAreaIC(const InputParameters & params);

  Real value(const Point & p) override;

protected:
  SubChannelMesh * _mesh;
};

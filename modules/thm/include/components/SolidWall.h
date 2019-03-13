#pragma once

#include "FlowBoundary.h"

class SolidWall;

template <>
InputParameters validParams<SolidWall>();

/**
 * A simple component for solid wall BC
 */
class SolidWall : public FlowBoundary
{
public:
  SolidWall(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void addMooseObjects1Phase();
  virtual void addMooseObjects2Phase();
  virtual void addMooseObjects2PhaseNCG();
};

#ifndef SOLIDWALL_H
#define SOLIDWALL_H

#include "FlowBoundary.h"

class SolidWall;

template <>
InputParameters validParams<SolidWall>();

/**
 * A simple component for solid wall BC
 *
 */
class SolidWall : public FlowBoundary
{
public:
  SolidWall(const InputParameters & params);

  virtual void addMooseObjects() override;

protected:
  virtual void check() const override;

  virtual void addMooseObjects1Phase();
  virtual void addMooseObjects2Phase();
};

#endif /* SOLIDWALL_H */

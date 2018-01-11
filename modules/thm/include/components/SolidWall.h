#ifndef SOLIDWALL_H
#define SOLIDWALL_H

#include "PipeBoundary.h"

class SolidWall;

template <>
InputParameters validParams<SolidWall>();

/**
 * A simple component for solid wall BC
 *
 */
class SolidWall : public PipeBoundary
{
public:
  SolidWall(const InputParameters & params);

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void addMooseObjects1Phase();
  virtual void addMooseObjects2Phase();
};

#endif /* SOLIDWALL_H */

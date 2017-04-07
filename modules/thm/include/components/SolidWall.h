#ifndef SOLIDWALL_H
#define SOLIDWALL_H

#include "BoundaryBase.h"

class SolidWall;

template <>
InputParameters validParams<SolidWall>();

/**
 * A simple component for solid wall BC
 *
 */
class SolidWall : public BoundaryBase
{
public:
  SolidWall(const InputParameters & params);
  virtual ~SolidWall();

  virtual void addVariables() override;
  virtual void addMooseObjects() override;

protected:
  virtual void addMooseObjects1Phase();
  virtual void addMooseObjects2Phase();
};

#endif /* SOLIDWALL_H */

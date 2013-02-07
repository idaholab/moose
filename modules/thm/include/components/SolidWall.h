#ifndef SOLIDWALL_H
#define SOLIDWALL_H

#include "BoundaryBase.h"

class SolidWall;

template<>
InputParameters validParams<SolidWall>();

/**
 * A simple component for solid wall BC
 *
 */
class SolidWall : public BoundaryBase
{
public:
  SolidWall(const std::string & name, InputParameters params);
  virtual ~SolidWall();

  virtual void addVariables();
  virtual void addMooseObjects();

protected:
};

#endif /* SOLIDWALL_H */

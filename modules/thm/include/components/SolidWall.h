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

  virtual void buildMesh();
  virtual void addVariables();
  virtual void addMooseObjects();

protected:
  std::string _input;                                   ///< Name of the input
  unsigned int _boundary_id;                            ///< boundary ID where is our BC imposed
};

#endif /* SOLIDWALL_H */

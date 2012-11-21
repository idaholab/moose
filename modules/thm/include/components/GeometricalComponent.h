#ifndef GEOMETRICALCOMPONENT_H
#define GEOMETRICALCOMPONENT_H

#include "Component.h"

class GeometricalComponent;

template<>
InputParameters validParams<GeometricalComponent>();

/**
 * Intermediate class for all geometrical components (i.e components that have position, direction, etc. in space - they generate a mesh)
 */
class GeometricalComponent : public Component
{
public:
  GeometricalComponent(const std::string & name, InputParameters parameters);
  virtual ~GeometricalComponent();

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }

protected:
  /// Physical position in the space
  Point _position;
  /// Direction this pipe is going to
  RealVectorValue _dir;

};

#endif /* GEOMETRICALCOMPONENT_H */

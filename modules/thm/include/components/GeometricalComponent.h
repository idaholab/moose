#ifndef GEOMETRICALCOMPONENT_H
#define GEOMETRICALCOMPONENT_H

#include "Component.h"
#include "Relap7App.h"
#include <map>

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

  virtual void doBuildMesh();
  virtual void displaceMesh();

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }
  virtual Real getRotation() { return _rotation; }

  virtual const std::vector<RELAP7::Connection> & getConnections(RELAP7::EEndType id) const;

protected:
  /// Node IDs
  unsigned int _first_node_id;
  unsigned int _last_node_id;
  /// Physical position in the space
  Point _position;
  /// Direction this pipe is going to
  RealVectorValue _dir;
  /// Rotation of the component around x-axis in non-displaced space
  Real _rotation;

  std::map<RELAP7::EEndType, std::vector<RELAP7::Connection> > _connections;
};

#endif /* GEOMETRICALCOMPONENT_H */

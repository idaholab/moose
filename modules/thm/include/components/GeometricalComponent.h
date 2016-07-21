#ifndef GEOMETRICALCOMPONENT_H
#define GEOMETRICALCOMPONENT_H

#include "Component.h"
#include "RELAP7App.h"
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
  GeometricalComponent(const InputParameters & parameters);
  virtual ~GeometricalComponent();

  virtual void doBuildMesh();
  virtual void displaceMesh();

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }
  virtual Real getRotation() { return _rotation; }

  virtual Real getNumElems() { return _n_elems; }
  virtual Real getLength() { return _length; }

  virtual const std::vector<RELAP7::Connection> & getConnections(RELAP7::EEndType id) const;

protected:
  const FunctionName & getVariableFn(const FunctionName & fn_param_name);

  /// Node IDs
  unsigned int _first_node_id;
  unsigned int _last_node_id;
  /// Physical position in the space
  Point _position;
  /// Offset for mesh generation
  Point _offset;
  /// Direction this pipe is going to
  RealVectorValue _dir;
  /// Rotation of the component around x-axis in non-displaced space
  Real _rotation;

  /// True if simulation is using a second order mesh
  bool _2nd_order_mesh;

  /// Length of the geometric component along the main axis
  Real _length;

  /// Number of elements along the main axis
  unsigned int _n_elems;

  /// Node locations along the main axis
  std::vector<Real> _node_locations;

  std::map<RELAP7::EEndType, std::vector<RELAP7::Connection> > _connections;

private:
  void processNodeLocations();
};

#endif /* GEOMETRICALCOMPONENT_H */

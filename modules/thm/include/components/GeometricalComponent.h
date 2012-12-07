#ifndef GEOMETRICALCOMPONENT_H
#define GEOMETRICALCOMPONENT_H

#include "Component.h"
#include "RELAP7.h"
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

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }

  virtual Node * getBoundaryNode(RELAP7::EEndType id);
  virtual unsigned int getBoundaryId(RELAP7::EEndType id);
  virtual Real getBoundaryOutNorm(RELAP7::EEndType id);
  virtual Real getArea(RELAP7::EEndType id) = 0;

protected:
  /// Physical position in the space
  Point _position;
  /// Direction this pipe is going to
  RealVectorValue _dir;

  /// Boundary nodes of this pipe (indexing: local "node id" => Node).
  /// Local node IDs are used by other components for connecting
  std::map<RELAP7::EEndType, Node *> _bnd_nodes;

  /// Boundary id of this pipe (indexing: local "node id" => boundary_id).
  std::map<RELAP7::EEndType, unsigned int> _bnd_ids;

  /// Out norm (either 1 or -1) on boundaries
  std::map<RELAP7::EEndType, Real> _bnd_out_norm;
};

#endif /* GEOMETRICALCOMPONENT_H */

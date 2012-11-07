#ifndef PIPEBASE_H
#define PIPEBASE_H

#include "point.h"
#include "vector_value.h"

#include "RELAP7.h"
#include "Component.h"
#include "Model.h"

class PipeBase;

template<>
InputParameters validParams<PipeBase>();

/**
 * A base class for pipe components
 *
 * A pipe is defined by its position, direction, length and area.
 * Mesh: mesh is generated in such a way, that the pipe starts at the origin (0, 0, 0) and is aligned with x-axis. Its
 * subdivided into _n_elems elements (of type EDGE2).
 */
class PipeBase : public Component,
    public Model
{
public:
  PipeBase(const std::string & name, InputParameters params);
  virtual ~PipeBase();

  virtual void buildMesh();
  virtual void addVariables();
  virtual void addMooseObjects();
  virtual const std::string & getType() { return _type; }

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }

  // Pipe specific interface ----
  virtual Node * getBoundaryNode(RELAP7::EEndType id);
  virtual unsigned int getBoundaryId(RELAP7::EEndType id);
  virtual int getBoundaryOutNorm(RELAP7::EEndType id);
  virtual Real getArea() { return _A; }
  virtual Real getLength() { return _length; }

protected:
  /// Physical position in the space
  Point _position;
  /// Offset for mesh generation
  Point _offset;
  /// Direction this pipe is going to
  RealVectorValue _dir;
  /// Length of the pipe
  Real _length;
  /// Number of elements this pipe is divided into
  unsigned int _n_elems;

  /// Pipe area (A_i)
  Real _A;
  /// Heat transfer surface density
  /// Physical meaning:
  ///   _aw = A_heating_surface_per_length / A_pipe_flow_area [1/m]
  Real _aw;
  /// Pipe hydraulic diameter, [m]
  Real _Dh;
  /// Roughness of pipe surface, [m]
  Real _roughness;
  /// a user-input shape factor for laminar friction factor for non-circular flow channels
  Real _shape_factor;

  /// True if user provides a constant f
  bool _has_f;
  /// Friction
  Real _f;
  /// True if user provides a constant Hw
  bool _has_Hw;
  /// Convective heat transfer coefficient
  Real _Hw;
  /// Wall temperature
  Real _Tw;
  /// Heat transfer geometry, represented by a number,
  /// use same code as RELAP5/3D for different geometry, could be changed to MooseEnum type
  unsigned int _HT_geometry_code;
  /// Pitch to diameter ratio for parallel bundle heat transfer
  Real  _PoD;
  /// True if user provides PoD
  bool _has_PoD;
  /// Gravity projected on the pipe direction
  Real _gx;
  /// Gravity constant, i.e., 9.8
  Real _g;

  /// Subdomain id this pipe defined
  unsigned int _subdomain_id;

  /// Nodes ids of this pipe component
  std::vector<unsigned int> node_ids;
  /// Elements ids of this pipe component
  std::vector<unsigned int> elem_ids;

  /// Boundary nodes of this pipe (indexing: local "node id" => Node).
  /// Local node IDs are used by other components for connecting
  std::map<RELAP7::EEndType, Node *> _bnd_nodes;

  /// Boundary id of this pipe (indexing: local "node id" => boundary_id).
  std::map<RELAP7::EEndType, unsigned int> _bnd_ids;

  /// Out norm (either 1 or -1) on boundaries
  std::map<RELAP7::EEndType, int> _bnd_out_norm;

  /// Is initial pressure provided from user input
  bool _has_initial_P;
  /// Is initial velocity provided from user input
  bool _has_initial_V;
  /// Is initial temperature provided from user input
  bool _has_initial_T;

  /// Initial pressure from user input (if provided)
  Real _initial_P;
  /// Initial velocity from user input (if provided)
  Real _initial_V;
  /// Initial temperature from user input (if provided)
  Real _initial_T;

public:
  static const std::string _type;
};

#endif /* PIPEBASE_H */

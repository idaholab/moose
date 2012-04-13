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
  virtual const std::string & getType() { return _type; }

  virtual Point getPosition() { return _position; }
  virtual RealVectorValue getDirection() { return _dir; }

  virtual std::vector<unsigned int> getIDs(std::string piece);
  virtual std::string variableName(std::string piece);

  // Pipe specific interface ----
  virtual Node * getBoundaryNode(RELAP7::EEndType id);
  virtual unsigned int getBoundaryId(RELAP7::EEndType id);
  virtual Real getArea() { return _A; }

protected:
  Point _position;                              ///< Physical position in the space
  /// Offset for mesh generation
  Point _offset;
  RealVectorValue _dir;                         ///< Direction this pipe is going to
  Real _length;                                 ///< Length of the pipe
  unsigned int _n_elems;                        ///< Number of elements this pipe is divided into

  Real _A;                                      ///< Pipe area (A_i)
  Real _aw;					///< Heat transfer surface density
						///< Physical meaning:
						///< _aw = A_heating_surface_per_length / A_pipe_flow_are [1/m]

  Real _f;                                      ///< friction
  Real _Hw;                                     ///< Convective heat transfer coefficient
  Real _Tw;                                     ///< Wall temperature


  unsigned int _subdomain_id;                   ///< Subdomain id this pipe defined

  std::vector<unsigned int> node_ids;		

  std::map<RELAP7::EEndType, Node *> _bnd_nodes;        ///< Boundary nodes of this pipe (indexing: local "node id" => Node).
                                                        ///< Local node IDs are used by other components for connecting

  std::map<RELAP7::EEndType, unsigned int> _bnd_ids;    ///< Boundary id of this pipe (indexing: local "node id" => boundary_id).

public:
  static const std::string _type;
};

#endif /* PIPEBASE_H */

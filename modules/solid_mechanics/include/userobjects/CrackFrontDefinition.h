#ifndef CRACKFRONTDEFINITION_H
#define CRACKFRONTDEFINITION_H

#include "GeneralUserObject.h"
#include "BoundaryRestrictable.h"
#include <set>

class CrackFrontDefinition;
class AuxiliarySystem;

template<>
InputParameters validParams<CrackFrontDefinition>();

/**
 * Works on top of NodalNormalsPreprocessor
 */
class CrackFrontDefinition :
  public GeneralUserObject,
  public BoundaryRestrictable
{
public:
  CrackFrontDefinition(const std::string & name, InputParameters parameters);
  virtual ~CrackFrontDefinition();

  virtual void initialSetup();
  virtual void initialize();
  virtual void finalize();
  virtual void execute();
  virtual void threadJoin(const UserObject & uo);

  void orderCrackFrontNodes(std::set<unsigned int> nodes);
  void orderEndNodes(std::vector<unsigned int> &end_nodes);
  void updateCrackFrontGeometry();

  const Node & getCrackFrontNode(const unsigned int node_index) const;
  const RealVectorValue & getCrackFrontTangent(const unsigned int node_index) const;
  Real getCrackFrontForwardSegmentLength(const unsigned int node_index) const;
  Real getCrackFrontBackwardSegmentLength(const unsigned int node_index) const;
  const RealVectorValue & getCrackDirection(const unsigned int node_index) const;
  bool treatAs2D() const {return _treat_as_2d;}

protected:
  AuxiliarySystem & _aux;
  MooseMesh & _mesh;

  std::vector<unsigned int> _ordered_crack_front_nodes;
  std::vector<RealVectorValue> _tangent_directions;
  std::vector<std::pair<Real,Real> > _segment_lengths;
  Real _overall_length;
  RealVectorValue _crack_direction;
  bool _treat_as_2d;
  unsigned int _axis_2d;
};


#endif /* CRACKFRONTDEFINITION_H */

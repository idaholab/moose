#ifndef CRACKFRONTDEFINITION_H
#define CRACKFRONTDEFINITION_H

#include "GeneralUserObject.h"
#include "BoundaryRestrictable.h"
#include <set>

class CrackFrontDefinition;
class AuxiliarySystem;

enum CDM_ENUM
{
  CDM_CRACK_DIRECTION_VECTOR=1,
  CDM_CRACK_MOUTH,
  CDM_CURVED_CRACK_FRONT
};

template<>
InputParameters validParams<CrackFrontDefinition>();
void addCrackFrontDefinitionParams(InputParameters& params);

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
  std::vector<RealVectorValue> _crack_directions;
  std::vector<std::pair<Real,Real> > _segment_lengths;
  Real _overall_length;
  CDM_ENUM _direction_method;
  RealVectorValue _crack_direction_vector;
  std::vector<BoundaryName> _crack_mouth_boundary_names;
  std::vector<BoundaryID> _crack_mouth_boundary_ids;
  RealVectorValue _crack_mouth_coordinates;
  RealVectorValue _crack_plane_normal_from_curved_front;
  bool _treat_as_2d;
  unsigned int _axis_2d;

  void getCrackFrontNodes(std::set<unsigned int>& nodes);
  void orderCrackFrontNodes(std::set<unsigned int> nodes);
  void orderEndNodes(std::vector<unsigned int> &end_nodes);
  void updateCrackFrontGeometry();
  void updateCrackDirectionCoords();
  RealVectorValue calculateCrackFrontDirection(const Node* crack_front_node,
                                               const RealVectorValue& tangent_direction) const;

};


#endif /* CRACKFRONTDEFINITION_H */

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRACKFRONTDEFINITION_H
#define CRACKFRONTDEFINITION_H

#include "GeneralUserObject.h"
#include "BoundaryRestrictable.h"
#include <set>
#include "SymmTensor.h"

class CrackFrontDefinition;
class AuxiliarySystem;

template <>
InputParameters validParams<CrackFrontDefinition>();
void addCrackFrontDefinitionParams(InputParameters & params);

/**
 * Works on top of NodalNormalsPreprocessor
 */
class CrackFrontDefinition : public GeneralUserObject, public BoundaryRestrictable
{
public:
  CrackFrontDefinition(const InputParameters & parameters);
  virtual ~CrackFrontDefinition();

  virtual void initialSetup();
  virtual void initialize();
  virtual void finalize();
  virtual void execute();

  const Node * getCrackFrontNodePtr(const unsigned int node_index) const;
  const Point * getCrackFrontPoint(const unsigned int point_index) const;
  const RealVectorValue & getCrackFrontTangent(const unsigned int point_index) const;
  Real getCrackFrontForwardSegmentLength(const unsigned int point_index) const;
  Real getCrackFrontBackwardSegmentLength(const unsigned int point_index) const;
  const RealVectorValue & getCrackDirection(const unsigned int point_index) const;
  Real getDistanceAlongFront(const unsigned int point_index) const;
  bool hasAngleAlongFront() const;
  Real getAngleAlongFront(const unsigned int point_index) const;
  unsigned int getNumCrackFrontPoints() const;
  bool treatAs2D() const { return _treat_as_2d; }
  RealVectorValue rotateToCrackFrontCoords(const RealVectorValue vector,
                                           const unsigned int point_index) const;
  ColumnMajorMatrix rotateToCrackFrontCoords(const SymmTensor tensor,
                                             const unsigned int point_index) const;
  ColumnMajorMatrix rotateToCrackFrontCoords(const ColumnMajorMatrix tensor,
                                             const unsigned int point_index) const;
  void calculateRThetaToCrackFront(const Point qp,
                                   const unsigned int point_index,
                                   Real & r,
                                   Real & theta) const;
  bool isNodeOnIntersectingBoundary(const Node * const node) const;
  bool isPointWithIndexOnIntersectingBoundary(const unsigned int point_index) const;
  Real getCrackFrontTangentialStrain(const unsigned int node_index) const;
  bool hasCrackFrontNodes() const { return _geom_definition_method == CRACK_FRONT_NODES; }
  bool isNodeInRing(const unsigned int ring_index,
                    const dof_id_type connected_node_id,
                    const unsigned int node_index) const;

protected:
  enum DIRECTION_METHOD
  {
    CRACK_DIRECTION_VECTOR,
    CRACK_MOUTH,
    CURVED_CRACK_FRONT
  };

  enum END_DIRECTION_METHOD
  {
    NO_SPECIAL_TREATMENT,
    END_CRACK_DIRECTION_VECTOR
  };

  enum CRACK_NODE_TYPE
  {
    MIDDLE_NODE,
    END_1_NODE,
    END_2_NODE
  };

  enum CRACK_GEOM_DEFINITION
  {
    CRACK_FRONT_NODES,
    CRACK_FRONT_POINTS
  };

  AuxiliarySystem & _aux;
  MooseMesh & _mesh;
  static const Real _tol;

  std::vector<unsigned int> _ordered_crack_front_nodes;
  CRACK_GEOM_DEFINITION _geom_definition_method;
  std::vector<Point> _crack_front_points;
  std::vector<RealVectorValue> _tangent_directions;
  std::vector<RealVectorValue> _crack_directions;
  std::vector<std::pair<Real, Real>> _segment_lengths;
  std::vector<Real> _distances_along_front;
  std::vector<Real> _angles_along_front;
  std::vector<Real> _strain_along_front;
  std::vector<ColumnMajorMatrix> _rot_matrix;
  Real _overall_length;
  DIRECTION_METHOD _direction_method;
  END_DIRECTION_METHOD _end_direction_method;
  RealVectorValue _crack_direction_vector;
  RealVectorValue _crack_direction_vector_end_1;
  RealVectorValue _crack_direction_vector_end_2;
  std::vector<BoundaryName> _crack_mouth_boundary_names;
  std::vector<BoundaryID> _crack_mouth_boundary_ids;
  std::vector<BoundaryName> _intersecting_boundary_names;
  std::vector<BoundaryID> _intersecting_boundary_ids;
  RealVectorValue _crack_mouth_coordinates;
  RealVectorValue _crack_plane_normal;
  bool _treat_as_2d;
  bool _closed_loop;
  unsigned int _axis_2d;
  bool _has_symmetry_plane;
  unsigned int _symmetry_plane;
  std::string _disp_x_var_name;
  std::string _disp_y_var_name;
  std::string _disp_z_var_name;
  bool _t_stress;
  bool _q_function_rings;
  unsigned int _last_ring;
  std::map<std::pair<dof_id_type, unsigned int>, std::set<dof_id_type>>
      _crack_front_node_to_node_map;

  void getCrackFrontNodes(std::set<dof_id_type> & nodes);
  void orderCrackFrontNodes(std::set<dof_id_type> & nodes);
  void orderEndNodes(std::vector<dof_id_type> & end_nodes);
  void
  pickLoopCrackEndNodes(std::vector<dof_id_type> & end_nodes,
                        std::set<dof_id_type> & nodes,
                        std::map<dof_id_type, std::vector<dof_id_type>> & node_to_line_elem_map,
                        std::vector<std::vector<dof_id_type>> & line_elems);
  unsigned int maxNodeCoor(std::vector<Node *> & nodes, unsigned int dir0 = 0);
  void updateCrackFrontGeometry();
  void updateDataForCrackDirection();
  RealVectorValue calculateCrackFrontDirection(const Point & crack_front_point,
                                               const RealVectorValue & tangent_direction,
                                               const CRACK_NODE_TYPE ntype) const;
  void calculateTangentialStrainAlongFront();
  void createQFunctionRings();
  void addNodesToQFunctionRing(std::set<dof_id_type> & nodes_new_ring,
                               const std::set<dof_id_type> & nodes_old_ring,
                               const std::set<dof_id_type> & nodes_all_rings,
                               const std::set<dof_id_type> & nodes_neighbor1,
                               const std::set<dof_id_type> & nodes_neighbor2,
                               std::vector<std::vector<const Elem *>> & nodes_to_elem_map);
};

#endif /* CRACKFRONTDEFINITION_H */

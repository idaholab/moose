/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALUSEROBJECTBASE_H
#define POLYCRYSTALUSEROBJECTBASE_H

#include "GeneralUserObject.h"
#include "Coupleable.h"

#include "libmesh/dense_matrix.h"

// Forward Declarations
class PolycrystalUserObjectBase;

template <>
InputParameters validParams<PolycrystalUserObjectBase>();

class PolycrystalUserObjectBase : public GeneralUserObject, public Coupleable
{
public:
  PolycrystalUserObjectBase(const InputParameters & parameters);

  // We do all of the work inside of the initialSetup method for this UserObject
  virtual void initialSetup() override;

  const std::map<dof_id_type, unsigned int> & getElemToGrainMap() const { return _elem_to_grain; }
  const MooseEnum & getColoringAlgorithm() const { return _coloring_algorithm; }

  /**
   * After the polycrystal data structure has been calculated, this callback will be
   * used to retrieve grain values for each element in the mesh.
   */
  virtual unsigned int getGrainID(dof_id_type elem_id) const = 0;

  virtual unsigned int getGrainBasedOnPoint(const Point & /*point*/) const { return 0; }

  static MooseEnum coloringAlgorithms();

  static std::string coloringAlgorithmDescriptions();

  static std::vector<unsigned int> assignOpsToGrains(DenseMatrix<Real> & adjacency_matrix,
                                                     unsigned int n_grains,
                                                     unsigned int n_ops,
                                                     const MooseEnum & coloring_algorithm);

  static bool colorGraph(const DenseMatrix<Real> & adjacency_matrix,
                         std::vector<unsigned int> & colors,
                         unsigned int n_vertices,
                         unsigned int n_colors,
                         unsigned int vertex);

  static bool isGraphValid(const DenseMatrix<Real> & adjacency_matrix,
                           std::vector<unsigned int> & colors,
                           unsigned int n_vertices,
                           unsigned int vertex,
                           unsigned int color);

protected:
  bool colorGraph(unsigned int vertex);

  bool isGraphValid(unsigned int vertex, unsigned int color);

  /// A reference to the mesh
  MooseMesh & _mesh;

  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries * _pb;

  /// mesh dimension
  unsigned int _dim;

  unsigned int _op_num;
  unsigned int _grain_num;

  MooseEnum _coloring_algorithm;

  std::vector<unsigned int> _grains_to_ops;
  bool _initialized;

  static const unsigned int INVALID_COLOR;
  static const unsigned int HALO_THICKNESS;

  /// The vector of coupled in variables
  std::vector<MooseVariable *> _vars;

  std::map<dof_id_type, unsigned int> _elem_to_grain;
};

#endif // POLYCRYSTALUSEROBJECTBASE_H

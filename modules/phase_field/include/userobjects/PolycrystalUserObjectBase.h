/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POLYCRYSTALUSEROBJECTBASE_H
#define POLYCRYSTALUSEROBJECTBASE_H

#include "FeatureFloodCount.h"

#include "libmesh/dense_matrix.h"

// Forward Declarations
class PolycrystalUserObjectBase;

template <>
InputParameters validParams<PolycrystalUserObjectBase>();

class PolycrystalUserObjectBase : public FeatureFloodCount
{
public:
  PolycrystalUserObjectBase(const InputParameters & parameters);

  virtual void precomputeGrainStructure() {}

  virtual unsigned int getGrainBasedOnPoint(const Point & point) const = 0;
  virtual unsigned int getGrainBasedOnElem(const Elem & elem) const
  {
    return getGrainBasedOnPoint(elem.centroid());
  }

  virtual void initialSetup() override;
  virtual void execute() override;
  virtual void finalize() override;

  bool isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                     std::size_t current_index,
                                     FeatureData *& feature,
                                     Status & status,
                                     unsigned int & new_id) override;

  virtual bool areFeaturesMergeable(const FeatureData & f1, const FeatureData & f2) const override;

  virtual const std::vector<unsigned int> & getGrainToOps() const { return _grain_to_op; }

  //  const std::map<dof_id_type, unsigned int> & getElemToGrainMap() const { return _elem_to_grain;
  //  }
  //  const MooseEnum & getColoringAlgorithm() const { return _coloring_algorithm; }

  /**
   * After the polycrystal data structure has been calculated, this callback will be
   * used to retrieve grain values for each element in the mesh.
   */
  //  virtual unsigned int getGrainID(dof_id_type elem_id) const = 0;

  static MooseEnum coloringAlgorithms();

  static std::string coloringAlgorithmDescriptions();

protected:
  void buildGrainAdjacencyMatrix();

  void assignOpsToGrains();

  bool colorGraph(unsigned int vertex);

  bool isGraphValid(unsigned int vertex, unsigned int color);

  std::unique_ptr<DenseMatrix<Real>> _adjacency_matrix;

  /// mesh dimension
  unsigned int _dim;

  unsigned int _op_num;
  unsigned int _grain_num;

  std::vector<unsigned int> _grain_to_op;

  MooseEnum _coloring_algorithm;
  bool _initialized;

  static const unsigned int INVALID_COLOR;
  static const unsigned int HALO_THICKNESS;

  //  std::map<dof_id_type, unsigned int> _elem_to_grain;
};

#endif // POLYCRYSTALUSEROBJECTBASE_H

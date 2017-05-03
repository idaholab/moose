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

/**
 * This object provides the base capability for creating proper polycrystal ICs. It is
 * able to discover the grain structure to provide information about neighboring grains
 * so that they will not be assigned the same order parameters with a reduced set of variables.
 */
class PolycrystalUserObjectBase : public FeatureFloodCount
{
public:
  PolycrystalUserObjectBase(const InputParameters & parameters);

  /**
   * This callback is triggered after the object is initialized and may be optionally
   * overridden to do precompute the element to grain identifiers ahead of time.
   */
  virtual void precomputeGrainStructure() {}

  /**
   * Method for retrieving the current grain ID based on some point in the mesh. Typically these
   * are element centroids or nodes depending on the basis functions being initialized. ICs that
   * have fixed resolution data (i.e. experimental datasets) may choose to implement
   * the element based method as well for added convenience.
   */
  virtual unsigned int getGrainBasedOnPoint(const Point & point) const = 0;

  /**
   * This method may be defined in addition to the point based initialization to speed up lookups.
   * It returns a grain based on the current element. Note: If your simulation contains adaptivity
   * the point based method may be used to retrieve grain information as well as this method.
   */
  virtual unsigned int getGrainBasedOnElem(const Elem & elem) const
  {
    return getGrainBasedOnPoint(elem.centroid());
  }

  /**
   * Method for retrieving the initial grain OP assignments.
   */
  virtual const std::vector<unsigned int> & getGrainToOps() const { return _grain_to_op; }

  /**
   * Returns all available coloring algorithms as an enumeration type for input files.
   */
  static MooseEnum coloringAlgorithms();

  /**
   * Returns corresponding descriptions of available coloring algorithms.
   */
  static std::string coloringAlgorithmDescriptions();

  /**
   * UserObject interface overrides. Derived classes should _not_ override any of these methods.
   */
  virtual void initialSetup() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  virtual bool areFeaturesMergeable(const FeatureData & f1, const FeatureData & f2) const override;
  virtual bool isNewFeatureOrConnectedRegion(const DofObject * dof_object,
                                             std::size_t current_index,
                                             FeatureData *& feature,
                                             Status & status,
                                             unsigned int & new_id) override;

  /**
   * Builds a dense adjacency matrix based on the discovery of grain neighbors and halos
   * surrounding each grain.
   */
  void buildGrainAdjacencyMatrix();

  /**
   * Method that runs a coloring algorithm to assign OPs to grains.
   */
  void assignOpsToGrains();

  /**
   * Built-in simple "back-tracking" algorithm to assign colors to a graph.
   */
  bool colorGraph(unsigned int vertex);

  /**
   * Helper method for the back-tracking graph coloring algorithm.
   */
  bool isGraphValid(unsigned int vertex, unsigned int color);

  /*************************************************
   *************** Data Structures *****************
   ************************************************/
  /// The dense adjacency matrix
  std::unique_ptr<DenseMatrix<Real>> _adjacency_matrix;

  /// mesh dimension
  const unsigned int _dim;

  /// The maximum number of order parameters (colors) available to assign to the grain structure
  const unsigned int _op_num;

  /// A vector indicating which op is assigned to each grain
  std::vector<unsigned int> _grain_to_op;

  /// The selected graph coloring algorithm used by this object
  const MooseEnum _coloring_algorithm;

  /// A Boolean indicating whether the object has been initialized or not (internal use)
  bool _initialized;

  /// A user controllable Boolean which can be used to print the adjacency matrix to the console
  const bool _output_adjacency_matrix;

  /// Used to indicate an invalid coloring for the built-in back-tracking algorithm
  static const unsigned int INVALID_COLOR;

  /// Used to hold the thickness of the halo that should be constructed for detecting adjacency
  static const unsigned int HALO_THICKNESS;
};

#endif // POLYCRYSTALUSEROBJECTBASE_H

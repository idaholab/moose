/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#pragma once

#include "ElementUserObject.h"
#include "DataIO.h"

#include "libmesh/data_type.h"

#include <set>
#include <map>
#include <vector>
#include <array>
#include <memory>
#include <tuple>

using namespace TIMPI;

class ThreadedRadialGreensConvolutionLoop;

/**
 * Gather and communicate a full list of all quadrature points and the values of
 * a selected variable at each point. Use a KD-Tree to integrate the weighted
 * neighborhood of each QP to obtain the convolution.
 */
class RadialGreensConvolution : public ElementUserObject
{
public:
  static InputParameters validParams();

  RadialGreensConvolution(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void meshChanged() override;

  /// quaddrature point data
  struct QPData
  {
    /// physical coordinates of the quadrature point
    Point _q_point;
    /// element id
    dof_id_type _elem_id;
    /// index of the quadrature point
    short _qp;
    /// current value * _JxW
    Real _volume;
    /// variable value
    Real _value;

    QPData() : _q_point(), _elem_id(libMesh::invalid_uint), _qp(0), _volume(0.0), _value(0.0) {}
    QPData(const Point & q_point, dof_id_type elem_id, short qp, Real volume, Real value)
      : _q_point(q_point), _elem_id(elem_id), _qp(qp), _volume(volume), _value(value)
    {
    }
  };

  using Result = std::map<dof_id_type, std::vector<Real>>;
  const Result & getConvolution() const { return _convolution; }

protected:
  Real attenuationIntegral(Real h1, Real h2, Real r, unsigned int dim) const;
  void insertNotLocalPointNeighbors(dof_id_type);
  void insertNotLocalPeriodicPointNeighbors(dof_id_type, const Node *);
  void findNotLocalPeriodicPointNeighbors(const Node *);
  void updateCommunicationLists();

  /// variable field to be gathered
  const VariableValue & _v;

  /// index of field variable
  unsigned int _v_var;

  /// Green's function
  const Function & _function;

  /// Green's function cut-off radius
  const Real _r_cut;

  /// Normalize the Green's function to one to make the integral of the convolution
  /// the same as the integral of the original data.
  const bool _normalize;

  /// mesh dimension
  unsigned int _dim;

  /// greens function integral table for correction of lower dimensional convolutions
  std::vector<Real> _correction_integral;

  /// gathered data
  std::vector<QPData> _qp_data;

  /// convolution result
  Result _convolution;

  /// is the mesh translated periodic in a given cardinal direction
  std::array<bool, LIBMESH_DIM> _periodic;

  ///@{ periodic size per component
  std::array<Real, LIBMESH_DIM> _periodic_min;
  std::array<Real, LIBMESH_DIM> _periodic_max;
  std::array<Point, LIBMESH_DIM> _periodic_vector;
  ///@}

  using KDTreeType = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<QPData>>,
      PointListAdaptor<QPData>,
      LIBMESH_DIM,
      std::size_t>;

  /// spatial index (nanoflann guarantees this to be threadsafe under read-only operations)
  std::unique_ptr<KDTreeType> _kd_tree;

  Real _zero_dh;

  /// DOF map
  const DofMap & _dof_map;

  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries * _pbs;

  /// PointLocator for finding topological neighbors
  std::unique_ptr<PointLocatorBase> _point_locator;

  /**
   * The data structure which is a list of nodes that are constrained to other nodes
   * based on the imposed periodic boundary conditions.
   */
  std::multimap<dof_id_type, dof_id_type> _periodic_node_map;

  /// The data structure used to find neighboring elements give a node ID
  std::vector<std::vector<const Elem *>> _nodes_to_elem_map;

  // list of direct point neighbor elements of the current processor domain
  std::set<const Elem *> _point_neighbors;

  // list of periodic point neighbor elements of the current processor domain
  std::set<std::tuple<const Elem *, const Node *, const Node *>> _periodic_point_neighbors;

  /// QPData indices to send to the various processors
  std::vector<std::set<std::size_t>> _communication_lists;
  bool _update_communication_lists;

  processor_id_type _my_pid;

  //@{ PerfGraph identifiers
  PerfID _perf_meshchanged;
  PerfID _perf_updatelists;
  PerfID _perf_finalize;
  //@}

  friend class ThreadedRadialGreensConvolutionLoop;
};

template <>
const Point & PointListAdaptor<RadialGreensConvolution::QPData>::getPoint(
    const RadialGreensConvolution::QPData & item) const;

namespace TIMPI
{

template <>
class StandardType<RadialGreensConvolution::QPData> : public DataType
{
public:
  explicit StandardType(const RadialGreensConvolution::QPData * example = nullptr);
  StandardType(const StandardType<RadialGreensConvolution::QPData> & t);
  ~StandardType() { this->free(); }
};
} // namespace TIMPI

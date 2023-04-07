//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "DataIO.h"
#include "PointListAdaptor.h"
#include "Function.h"

#include "libmesh/data_type.h"

#include <set>
#include <map>
#include <vector>
#include <memory>

class ThreadedRadialAverageLoop;

/**
 * Gather and communicate a full list of all quadrature points and the values of
 * a selected material property at each point. Use a KD-Tree to get the weighted spatial
 * average of a material property. This code borrows heavily from
 * RadialGreensConvolution in MAGPIE. This code does not include support for
 * periodic BCs, but RadialGreensConvolution shows how that can be supported.
 */
class RadialAverage : public ElementUserObject
{
public:
  static InputParameters validParams();

  RadialAverage(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void meshChanged() override;

  /// quadrature point data
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
  const Result & getAverage() const { return _average; }

protected:
  void updateCommunicationLists();

  /// distance based weight function
  enum class WeightsType
  {
    CONSTANT,
    LINEAR,
    COSINE
  } _weights_type;

  /// material to be averaged
  const MaterialProperty<Real> & _prop;

  /// cut-off radius
  const Real _radius;

  /// communication padding distance
  const Real _padding;

  /// gathered data
  std::vector<QPData> _qp_data;

  /// average result
  Result _average;

  using KDTreeType = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<QPData>>,
      PointListAdaptor<QPData>,
      LIBMESH_DIM,
      std::size_t>;

  /// spatial index (nanoflann guarantees this to be threadsafe under read-only operations)
  std::unique_ptr<KDTreeType> _kd_tree;

  /// The data structure used to find neighboring elements give a node ID
  std::vector<std::vector<const Elem *>> _nodes_to_elem_map;

  /// set of nodes on the boundary of the current processor domain
  std::set<Point> _boundary_nodes;

  /// set of all _qp_data indices that are within _radius of any _boundary_nodes
  std::set<std::size_t> _boundary_data_indices;

  /// QPData indices to send to the various processors
  std::vector<std::set<std::size_t>> _communication_lists;
  bool _update_communication_lists;

  /// processors to send (potentially empty) data to
  std::vector<processor_id_type> _candidate_procs;

  processor_id_type _my_pid;

  //@{ PerfGraph identifiers
  PerfID _perf_meshchanged;
  PerfID _perf_updatelists;
  PerfID _perf_finalize;
  //@}

  friend class ThreadedRadialAverageLoop;
};

namespace TIMPI
{

template <>
class StandardType<RadialAverage::QPData> : public DataType
{
public:
  explicit StandardType(const RadialAverage::QPData * example = nullptr);
  StandardType(const StandardType<RadialAverage::QPData> & t);
  ~StandardType() { this->free(); }
};

} // namespace TIMPI

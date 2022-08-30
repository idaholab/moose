//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RadialAverage.h"
#include "ThreadedRadialAverageLoop.h"
#include "FEProblemBase.h"

#include "libmesh/nanoflann.hpp"
#include "libmesh/parallel_algebra.h"
#include "libmesh/mesh_tools.h"
#include "libmesh/int_range.h"

#include <list>
#include <iterator>
#include <algorithm>

registerMooseObject("MooseApp", RadialAverage);

// specialization for PointListAdaptor<RadialAverage::QPData>
template <>
const Point &
PointListAdaptor<RadialAverage::QPData>::getPoint(const RadialAverage::QPData & item) const
{
  return item._q_point;
}

InputParameters
RadialAverage::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Perform a radial average of a material property");
  params.addRequiredParam<MaterialPropertyName>("prop_name",
                                                "Name of the material property to average");
  MooseEnum weights_type("constant linear cosine", "linear");
  params.addRequiredParam<MooseEnum>("weights", weights_type, "Distance based weight function");
  params.addRequiredParam<Real>("radius", "Cut-off radius for the averaging");
  params.addRangeCheckedParam<Real>(
      "padding",
      0.0,
      "padding >= 0",
      "Padding for communication. This gets added to the radius when determining which QPs to send "
      "to other processors. Increase this gradually if inconsistent parallel results occur.");

  // we run this object once at the beginning of the timestep by default
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  // make sure we always have geometric point neighbors ghosted
  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  params.addParamNamesToGroup("padding", "Advanced");
  return params;
}

RadialAverage::RadialAverage(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _weights_type(getParam<MooseEnum>("weights").getEnum<WeightsType>()),
    _prop(getMaterialProperty<Real>("prop_name")),
    _radius(getParam<Real>("radius")),
    _padding(getParam<Real>("padding")),
    _update_communication_lists(false),
    _my_pid(processor_id()),
    _perf_meshchanged(registerTimedSection("meshChanged", 3)),
    _perf_updatelists(registerTimedSection("updateCommunicationLists", 3)),
    _perf_finalize(registerTimedSection("finalize", 2))
{
}

void
RadialAverage::initialSetup()
{
  // set up processor boundary node list
  meshChanged();
}

void
RadialAverage::initialize()
{
  _qp_data.clear();
}

void
RadialAverage::execute()
{
  auto id = _current_elem->id();

  // collect all QP data
  for (const auto qp : make_range(_qrule->n_points()))
    _qp_data.emplace_back(_q_point[qp], id, qp, _JxW[qp] * _coord[qp], _prop[qp]);

  // make sure the result map entry for the current element is sized correctly
  auto i = _average.find(id);
  if (i == _average.end())
    _average.insert(std::make_pair(id, std::vector<Real>(_qrule->n_points())));
  else
    i->second.resize(_qrule->n_points());
}

void
RadialAverage::finalize()
{
  TIME_SECTION(_perf_finalize);

  // the first chunk of data is always the local data - remember its size
  unsigned int local_size = _qp_data.size();

  // communicate the qp data list if n_proc > 1
  if (_app.n_processors() > 1)
  {
    // !!!!!!!!!!!
    // !!CAREFUL!! Is it guaranteed that _qp_data is in the same order if the mesh has not changed?
    // According to @friedmud it is not guaranteed if threads are used
    // !!!!!!!!!!!

    // update after mesh changes and/or if a displaced problem exists
    if (_update_communication_lists || _fe_problem.getDisplacedProblem() ||
        libMesh::n_threads() > 1)
      updateCommunicationLists();

    // data structures for sparse point to point communication
    std::vector<std::vector<QPData>> send(_candidate_procs.size());
    std::vector<Parallel::Request> send_requests(_candidate_procs.size());
    Parallel::MessageTag send_tag = _communicator.get_unique_tag(4711);
    std::vector<QPData> receive;

    const auto item_type = TIMPI::StandardType<QPData>(&(_qp_data[0]));

    // fill buffer and send structures
    for (const auto i : index_range(_candidate_procs))
    {
      const auto pid = _candidate_procs[i];
      const auto & list = _communication_lists[pid];

      // fill send buffer for transfer to pid
      send[i].reserve(list.size());
      for (const auto & item : list)
        send[i].push_back(_qp_data[item]);

      // issue non-blocking send
      _communicator.send(pid, send[i], send_requests[i], send_tag);
    }

    // receive messages - we assume that we receive as many messages as we send!
    // bounding box overlapp is transitive, but data exhange between overlapping procs could still
    // be unidirectional!
    for (const auto i : index_range(_candidate_procs))
    {
      libmesh_ignore(i);

      // inspect incoming message
      Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag));
      const auto source_pid = TIMPI::cast_int<processor_id_type>(status.source());
      const auto message_size = status.size(item_type);

      // resize receive buffer accordingly and receive data
      receive.resize(message_size);
      _communicator.receive(source_pid, receive, send_tag);

      // append communicated data
      _qp_data.insert(_qp_data.end(), receive.begin(), receive.end());
    }

    // wait until all send requests are at least buffered and we can destroy
    // the send buffers by going out of scope
    Parallel::wait(send_requests);
  }

  // build KD-Tree using data we just received
  const unsigned int max_leaf_size = 20; // slightly affects runtime
  auto point_list = PointListAdaptor<QPData>(_qp_data.begin(), _qp_data.end());
  _kd_tree = std::make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));

  mooseAssert(_kd_tree != nullptr, "KDTree was not properly initialized.");
  _kd_tree->buildIndex();

  // build thread loop functor
  ThreadedRadialAverageLoop rgcl(*this);

  // run threads
  auto local_range_begin = _qp_data.begin();
  auto local_range_end = local_range_begin;
  std::advance(local_range_end, local_size);
  Threads::parallel_reduce(QPDataRange(local_range_begin, local_range_end), rgcl);
}

void
RadialAverage::threadJoin(const UserObject & y)
{
  const RadialAverage & uo = static_cast<const RadialAverage &>(y);
  _qp_data.insert(_qp_data.begin(), uo._qp_data.begin(), uo._qp_data.end());
  _average.insert(uo._average.begin(), uo._average.end());
}

void
RadialAverage::meshChanged()
{
  TIME_SECTION(_perf_meshchanged);

  // get underlying libMesh mesh
  auto & mesh = _mesh.getMesh();

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  // clear procesor boundary nodes set
  _boundary_nodes.clear();

  //
  // iterate over active local elements and store all processor boundary node locations
  //
  const auto end = mesh.active_local_elements_end();
  for (auto it = mesh.active_local_elements_begin(); it != end; ++it)
    // find faces at processor boundaries
    for (const auto s : make_range((*it)->n_sides()))
    {
      const auto * neighbor = (*it)->neighbor_ptr(s);
      if (neighbor && neighbor->processor_id() != _my_pid)
        // add all nodes on the processor boundary
        for (const auto n : make_range((*it)->n_nodes()))
          if ((*it)->is_node_on_side(n, s))
            _boundary_nodes.insert((*it)->node_ref(n));

      // request communication list update
      _update_communication_lists = true;
    }
}

void
RadialAverage::updateCommunicationLists()
{
  TIME_SECTION(_perf_updatelists);

  // clear communication lists
  _communication_lists.clear();
  _communication_lists.resize(n_processors());

  // build KD-Tree using local qpoint data
  const unsigned int max_leaf_size = 20; // slightly affects runtime
  auto point_list = PointListAdaptor<QPData>(_qp_data.begin(), _qp_data.end());
  auto kd_tree = std::make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));
  mooseAssert(kd_tree != nullptr, "KDTree was not properly initialized.");
  kd_tree->buildIndex();

  std::vector<std::pair<std::size_t, Real>> ret_matches;
  nanoflann::SearchParams search_params;

  // iterate over all boundary nodes and collect all boundary-near data points
  _boundary_data_indices.clear();
  for (const auto & bn : _boundary_nodes)
  {
    ret_matches.clear();
    kd_tree->radiusSearch(
        &(bn(0)), Utility::pow<2>(_radius + _padding), ret_matches, search_params);
    for (auto & match : ret_matches)
      _boundary_data_indices.insert(match.first);
  }

  // gather all processor bounding boxes (communicate as pairs)
  std::vector<std::pair<Point, Point>> pps(n_processors());
  const auto mybb = _mesh.getInflatedProcessorBoundingBox(0);
  std::pair<Point, Point> mypp = mybb;
  _communicator.allgather(mypp, pps);

  // inflate all processor bounding boxes by radius (no padding)
  const auto rpoint = Point(1, 1, 1) * _radius;
  std::vector<BoundingBox> bbs;
  for (const auto & pp : pps)
    bbs.emplace_back(pp.first - rpoint, pp.second + rpoint);

  // get candidate processors (overlapping bounding boxes)
  _candidate_procs.clear();
  for (const auto pid : index_range(bbs))
    if (pid != _my_pid && bbs[pid].intersects(mypp))
      _candidate_procs.push_back(pid);

  // go over all boundary data items and send them to the proc they overlap with
  for (const auto i : _boundary_data_indices)
    for (const auto pid : _candidate_procs)
      if (bbs[pid].contains_point(_qp_data[i]._q_point))
        _communication_lists[pid].insert(i);

  // done
  _update_communication_lists = false;
}

namespace TIMPI
{

StandardType<RadialAverage::QPData>::StandardType(const RadialAverage::QPData * example)
{
  // We need an example for MPI_Address to use
  static const RadialAverage::QPData p;
  if (!example)
    example = &p;

#ifdef LIBMESH_HAVE_MPI

  // Get the sub-data-types, and make sure they live long enough
  // to construct the derived type
  StandardType<Point> d1(&example->_q_point);
  StandardType<dof_id_type> d2(&example->_elem_id);
  StandardType<short> d3(&example->_qp);
  StandardType<Real> d4(&example->_volume);
  StandardType<Real> d5(&example->_value);

  MPI_Datatype types[] = {
      (data_type)d1, (data_type)d2, (data_type)d3, (data_type)d4, (data_type)d5};
  int blocklengths[] = {1, 1, 1, 1, 1};
  MPI_Aint displs[5], start;

  libmesh_call_mpi(MPI_Get_address(const_cast<RadialAverage::QPData *>(example), &start));
  libmesh_call_mpi(MPI_Get_address(const_cast<Point *>(&example->_q_point), &displs[0]));
  libmesh_call_mpi(MPI_Get_address(const_cast<dof_id_type *>(&example->_elem_id), &displs[1]));
  libmesh_call_mpi(MPI_Get_address(const_cast<short *>(&example->_qp), &displs[2]));
  libmesh_call_mpi(MPI_Get_address(const_cast<Real *>(&example->_volume), &displs[3]));
  libmesh_call_mpi(MPI_Get_address(const_cast<Real *>(&example->_value), &displs[4]));

  for (std::size_t i = 0; i < 5; ++i)
    displs[i] -= start;

  // create a prototype structure
  MPI_Datatype tmptype;
  libmesh_call_mpi(MPI_Type_create_struct(5, blocklengths, displs, types, &tmptype));
  libmesh_call_mpi(MPI_Type_commit(&tmptype));

  // resize the structure type to account for padding, if any
  libmesh_call_mpi(MPI_Type_create_resized(tmptype, 0, sizeof(RadialAverage::QPData), &_datatype));
  libmesh_call_mpi(MPI_Type_free(&tmptype));

  this->commit();

#endif // LIBMESH_HAVE_MPI
}

StandardType<RadialAverage::QPData>::StandardType(const StandardType<RadialAverage::QPData> & t)
  : DataType(t._datatype)
{
#ifdef LIBMESH_HAVE_MPI
  libmesh_call_mpi(MPI_Type_dup(t._datatype, &_datatype));
#endif
}

} // namespace TIMPI

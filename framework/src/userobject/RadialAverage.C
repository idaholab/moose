/**********************************************************************/
/*                     DO NOT MODIFY THIS HEADER                      */
/* MAGPIE - Mesoscale Atomistic Glue Program for Integrated Execution */
/*                                                                    */
/*            Copyright 2017 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "RadialGreensConvolution.h"
#include "ThreadedRadialGreensConvolutionLoop.h"
#include "NonlinearSystemBase.h"
#include "FEProblemBase.h"

#include "libmesh/nanoflann.hpp"
#include "libmesh/parallel_algebra.h"
#include "libmesh/mesh_tools.h"

#include <list>
#include <iterator>
#include <algorithm>

registerMooseObject("MagpieApp", RadialGreensConvolution);

// specialization for PointListAdaptor<RadialGreensConvolution::QPData>
template <>
const Point &
PointListAdaptor<RadialGreensConvolution::QPData>::getPoint(
    const RadialGreensConvolution::QPData & item) const
{
  return item._q_point;
}

InputParameters
RadialGreensConvolution::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addClassDescription("Perform a radial Green's function convolution");
  params.addCoupledVar("v", "Variable to gather");
  params.addRequiredParam<FunctionName>(
      "function",
      "Green's function (distance is substituted for x) without geometrical attenuation");
  params.addRequiredParam<Real>("r_cut", "Cut-off radius for the Green's function");
  params.addParam<bool>("normalize", false, "Normalize the Green's function integral to one");

  // we run this object once at the beginning of the timestep by default
  params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN;

  // make sure we always have geometric point neighbors ghosted
  params.addRelationshipManager("ElementPointNeighborLayers",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);

  return params;
}

RadialGreensConvolution::RadialGreensConvolution(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _v(coupledValue("v")),
    _v_var(coupled("v")),
    _function(getFunction("function")),
    _r_cut(getParam<Real>("r_cut")),
    _normalize(getParam<bool>("normalize")),
    _dim(_mesh.dimension()),
    _correction_integral(100),
    _dof_map(_fe_problem.getNonlinearSystemBase().dofMap()),
    _update_communication_lists(false),
    _my_pid(processor_id()),
    _perf_meshchanged(registerTimedSection("meshChanged", 3)),
    _perf_updatelists(registerTimedSection("updateCommunicationLists", 3)),
    _perf_finalize(registerTimedSection("finalize", 2))
{
  // collect mesh periodicity data
  for (unsigned int i = 0; i < _dim; ++i)
  {
    _periodic[i] = _mesh.isRegularOrthogonal() && _mesh.isTranslatedPeriodic(_v_var, i);

    _periodic_min[i] = _mesh.getMinInDimension(i);
    _periodic_max[i] = _mesh.getMaxInDimension(i);
    _periodic_vector[i](i) = _mesh.dimensionWidth(i);

    // we could allow this, but then we'd have to search over more than just the nearest periodic
    // neighbors
    if (_periodic[i] && 2.0 * _r_cut > _periodic_vector[i](i))
      paramError(
          "r_cut",
          "The cut-off radius cannot be larger than half the periodic size of the simulation cell");
  }
}

void
RadialGreensConvolution::initialSetup()
{
  // Get a pointer to the PeriodicBoundaries buried in libMesh
  _pbs = _fe_problem.getNonlinearSystemBase().dofMap().get_periodic_boundaries();

  // set up processor boundary node list
  meshChanged();
}

void
RadialGreensConvolution::initialize()
{
  _qp_data.clear();

  /* compute the dimensional correction
   *
   *  |---__
   *  |     / .
   *  |  R/  | .
   *  | /   h|  \ r_cut
   *  |______|__|____
   *         r
   *
   * Out of plane (2D) and out of line (1D) 3D contributions to the convolution
   * are not naturally captured by the 1D or 2D numerical integration of QP neighborhood.
   * At every Qp an integration in the h direction needs to be performed.
   * We can pre-tabulate this integral.
   */
  if (_dim < 3)
  {
    const Real dr = _r_cut / _correction_integral.size();
    for (unsigned int i = 0; i < _correction_integral.size(); ++i)
    {
      const Real r = i * dr;
      const Real h = std::sqrt(_r_cut * _r_cut - r * r);
      const unsigned int hsteps = std::ceil(h / dr);
      const Real dh = h / hsteps;

      _correction_integral[i] = 0.0;

      // for r = 0 we skip the first bin of the integral!
      if (i == 0)
        _zero_dh = dh;

      for (unsigned int j = i > 0 ? 0 : 1; j < hsteps; ++j)
      {
        const Real h1 = j * dh;
        const Real h2 = h1 + dh;

        // we evaluate the Green's function at the geometric middle of the height interval
        // and assume it to be constant.
        const Real R2 = r * r + h1 * h2;
        const Real G = _function.value(_t, Point(std::sqrt(R2), 0.0, 0.0));

        // We integrate the geometric attenuation analytically over the interval [h1, h2).
        _correction_integral[i] += G * attenuationIntegral(h1, h2, r, _dim);
      }
    }
  }
}

Real
RadialGreensConvolution::attenuationIntegral(Real h1, Real h2, Real r, unsigned int dim) const
{
  if (dim == 2)
  {
    // in 2D we need to return the above and below plane contribution (hence 2.0 * ...)
    if (r > 0.0)
      // integral 1/(h^2+r^2) dh = 1/r*atan(h/r)|
      return 2.0 * 1.0 / (4.0 * libMesh::pi * r) * (std::atan(h2 / r) - std::atan(h1 / r));
    else
      // integral 1/h^2 dh = -1/h|
      return 2.0 * 1.0 / (4.0 * libMesh::pi) * (-1.0 / h2 + 1.0 / h1);
  }
  else
    // dim = 1, we multiply the attenuation by 2pi*h (1/2 * 2pi/4pi = 0.25)
    // integral h/(h^2+r^2) dh = 1/2 * ln(h^2+r^2)|
    return 0.25 * (std::log(h2 * h2 + r * r) - std::log(h1 * h1 + r * r));
}

void
RadialGreensConvolution::execute()
{
  auto id = _current_elem->id();

  // collect all QP data
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    _qp_data.emplace_back(_q_point[qp], id, qp, _JxW[qp] * _coord[qp], _v[qp]);

  // make sure the result map entry for the current element is sized correctly
  auto i = _convolution.find(id);
  if (i == _convolution.end())
    _convolution.insert(std::make_pair(id, std::vector<Real>(_qrule->n_points())));
  else
    i->second.resize(_qrule->n_points());
}

void
RadialGreensConvolution::finalize()
{
  TIME_SECTION(_perf_finalize);

  // the first chunk of data is always the local data - remember its size
  unsigned int local_size = _qp_data.size();

  // if normalization is requested we need to integrate the current variable field
  Real source_integral = 0.0;
  if (_normalize)
  {
    for (const auto & qpd : _qp_data)
      source_integral += qpd._volume * qpd._value;
    gatherSum(source_integral);
  }

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

    // sparse send data (processor ID,)
    std::vector<std::size_t> non_zero_comm;
    for (auto i = beginIndex(_communication_lists); i < _communication_lists.size(); ++i)
      if (!_communication_lists[i].empty())
        non_zero_comm.push_back(i);

    // data structures for sparse point to point communication
    std::vector<std::vector<QPData>> send(non_zero_comm.size());
    std::vector<Parallel::Request> send_requests(non_zero_comm.size());
    Parallel::MessageTag send_tag = _communicator.get_unique_tag(4711);
    std::vector<QPData> receive;

    const auto item_type = StandardType<QPData>(&(_qp_data[0]));

#if 0
    // output local qp locations
    // _console << name() << ' ' << receive.size() << '\n' << name() << std::flush;
    for (auto item : _qp_data)
      _console << name() << ' ' << _my_pid << ' '
               << item._q_point(0) << ' '
               << item._q_point(1) << ' '
               << item._q_point(2) << std::flush;
#endif

    // fill buffer and send structures
    for (auto i = beginIndex(non_zero_comm); i < non_zero_comm.size(); ++i)
    {
      const auto pid = non_zero_comm[i];
      const auto & list = _communication_lists[pid];

      // fill send buffer for transfer to pid
      send[i].reserve(list.size());
      for (const auto & item : list)
      {
        send[i].push_back(_qp_data[item]);
#if 0
        // output sent qp locations
        _console << name() << ' '
                 << _qp_data[item]._q_point(0) << ' '
                 << _qp_data[item]._q_point(1) << ' '
                 << _qp_data[item]._q_point(2) << ' '
                 << pid << std::flush;
#endif
      }

      // issue non-blocking send
      _communicator.send(pid, send[i], send_requests[i], send_tag);
    }

    // receive messages - we assume that we receive as many messages as we send!
    for (auto i = beginIndex(non_zero_comm); i < non_zero_comm.size(); ++i)
    {
      // inspect incoming message
      Parallel::Status status(_communicator.probe(Parallel::any_source, send_tag));
      const auto source_pid = TIMPI::cast_int<processor_id_type>(status.source());
      const auto message_size = status.size(item_type);

      // resize receive buffer accordingly and receive data
      receive.resize(message_size);
      _communicator.receive(source_pid, receive, send_tag);

#if 0
      // output received qp locations
      // _console << name() << ' ' << receive.size() << '\n' << name() << std::flush;
      for (auto item : receive)
        _console << name() << ' ' << source_pid << ' '
                 << item._q_point(0) << ' '
                 << item._q_point(1) << ' '
                 << item._q_point(2) << std::flush;
#endif

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
  _kd_tree = libmesh_make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));

  mooseAssert(_kd_tree != nullptr, "KDTree was not properly initialized.");
  _kd_tree->buildIndex();

  // result map entry
  const auto end_it = _convolution.end();
  auto it = end_it;

  // build thread loop functor
  ThreadedRadialGreensConvolutionLoop rgcl(*this);

  // run threads
  auto local_range_begin = _qp_data.begin();
  auto local_range_end = local_range_begin;
  std::advance(local_range_end, local_size);
  Threads::parallel_reduce(QPDataRange(local_range_begin, local_range_end), rgcl);

  Real convolution_integral = rgcl.convolutionIntegral();

  // if normalization is requested we need to communicate the convolution result
  // and normalize the result entries
  if (_normalize)
  {
    gatherSum(convolution_integral);

    // we may not need to or may not be able to normalize
    if (convolution_integral == 0.0)
    {
      if (source_integral == 0.0)
        return;
      mooseError("Unable to normalize Green's function. Is it all zero?");
    }

    // normalize result entries
    for (auto & re : _convolution)
      for (auto & ri : re.second)
        ri *= source_integral / convolution_integral;
  }

  // make it a differential result
  it = end_it;
  for (unsigned int i = 0; i < local_size; ++i)
  {
    const auto & local_qp = _qp_data[i];

    // Look up result map iterator only if we enter a new element. this saves a bunch
    // of map lookups because same element entries are consecutive in the _qp_data vector.
    if (it == end_it || it->first != local_qp._elem_id)
      it = _convolution.find(local_qp._elem_id);

    it->second[local_qp._qp] -= local_qp._volume * local_qp._value;
  }
}

void
RadialGreensConvolution::threadJoin(const UserObject & y)
{
  const RadialGreensConvolution & uo = static_cast<const RadialGreensConvolution &>(y);
  _qp_data.insert(_qp_data.begin(), uo._qp_data.begin(), uo._qp_data.end());
  _convolution.insert(uo._convolution.begin(), uo._convolution.end());
}

void
RadialGreensConvolution::insertNotLocalPointNeighbors(dof_id_type node)
{
  mooseAssert(!_nodes_to_elem_map[node].empty(), "Node not found in _nodes_to_elem_map");

  for (const auto * elem : _nodes_to_elem_map[node])
    if (elem->processor_id() != _my_pid)
      _point_neighbors.insert(elem);
}

void
RadialGreensConvolution::insertNotLocalPeriodicPointNeighbors(dof_id_type node,
                                                              const Node * reference)
{
  mooseAssert(!_nodes_to_elem_map[node].empty(), "Node not found in _nodes_to_elem_map");

  const Node * first = _mesh.nodePtr(node);
  for (const auto * elem : _nodes_to_elem_map[node])
    if (elem->processor_id() != _my_pid)
      _periodic_point_neighbors.emplace(elem, first, reference);
}

void
RadialGreensConvolution::findNotLocalPeriodicPointNeighbors(const Node * first)
{
  auto iters = _periodic_node_map.equal_range(first->id());
  auto it = iters.first;

  // no periodic copies
  if (it == iters.second)
    return;

  // insert first periodic neighbor
  insertNotLocalPeriodicPointNeighbors(it->second, first);
  ++it;

  // usual case, node was on the face of a periodic boundary (and not on its edge or corner)
  if (it == iters.second)
    return;

  // number of periodic directions
  unsigned int periodic_dirs = 1;
  std::set<dof_id_type> nodes;
  nodes.insert(iters.first->second); // probably not necessary

  // insert remaining periodic copies
  do
  {
    nodes.insert(it->second);
    it++;
    periodic_dirs++;
  } while (it != iters.second);

  // now jump periodic_dirs-1 times from those nodes to their periodic copies
  for (unsigned int i = 1; i < periodic_dirs; ++i)
  {
    std::set<dof_id_type> new_nodes;
    for (auto node : nodes)
    {
      // periodic copies of the set members we already inserted
      auto new_iters = _periodic_node_map.equal_range(node);

      // insert the ids of the periodic copies of the node
      for (it = new_iters.first; it != new_iters.second; ++it)
        new_nodes.insert(it->second);
    }
    nodes.insert(new_nodes.begin(), new_nodes.end());
  }

  // add all jumped to nodes
  for (auto node : nodes)
    insertNotLocalPeriodicPointNeighbors(node, first);
}

void
RadialGreensConvolution::meshChanged()
{
  TIME_SECTION(_perf_meshchanged);

  // get underlying libMesh mesh
  auto & mesh = _mesh.getMesh();

  // get a fresh point locator
  _point_locator = _mesh.getPointLocator();

  // rebuild periodic node map (this is the heaviest part by far)
  _mesh.buildPeriodicNodeMap(_periodic_node_map, _v_var, _pbs);

  // Build a new node to element map
  _nodes_to_elem_map.clear();
  MeshTools::build_nodes_to_elem_map(_mesh.getMesh(), _nodes_to_elem_map);

  // clear point neighbor set
  _point_neighbors.clear();

  // my processor id
  const auto my_pid = processor_id();

  // iterate over active local elements
  const auto end = mesh.active_local_elements_end();
  for (auto it = mesh.active_local_elements_begin(); it != end; ++it)
    // find a face that faces either a boundary (nullptr) or a different processor
    for (unsigned int s = 0; s < (*it)->n_sides(); ++s)
    {
      const auto * neighbor = (*it)->neighbor_ptr(s);
      if (neighbor)
      {
        if (neighbor->processor_id() != my_pid)
        {
          // add all face node touching elements directly
          for (unsigned int n = 0; n < (*it)->n_nodes(); ++n)
            if ((*it)->is_node_on_side(n, s))
              insertNotLocalPointNeighbors((*it)->node_id(n));
        }
      }
      else
      {
        // find periodic node neighbors and
        for (unsigned int n = 0; n < (*it)->n_nodes(); ++n)
          if ((*it)->is_node_on_side(n, s))
            findNotLocalPeriodicPointNeighbors((*it)->node_ptr(n));
      }
    }

  // request communication list update
  _update_communication_lists = true;
}

void
RadialGreensConvolution::updateCommunicationLists()
{
  TIME_SECTION(_perf_updatelists);

  // clear communication lists
  _communication_lists.clear();
  _communication_lists.resize(n_processors());

  // build KD-Tree using local qpoint data
  const unsigned int max_leaf_size = 20; // slightly affects runtime
  auto point_list = PointListAdaptor<QPData>(_qp_data.begin(), _qp_data.end());
  auto kd_tree = libmesh_make_unique<KDTreeType>(
      LIBMESH_DIM, point_list, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size));
  mooseAssert(kd_tree != nullptr, "KDTree was not properly initialized.");
  kd_tree->buildIndex();

  std::vector<std::pair<std::size_t, Real>> ret_matches;
  nanoflann::SearchParams search_params;

  // iterate over non periodic point neighbor elements
  for (auto elem : _point_neighbors)
  {
    ret_matches.clear();
    Point centroid = elem->centroid();
    const Real r_cut2 = _r_cut + elem->hmax() / 2.0;
    kd_tree->radiusSearch(&(centroid(0)), r_cut2 * r_cut2, ret_matches, search_params);
    for (auto & match : ret_matches)
      _communication_lists[elem->processor_id()].insert(match.first);
  }

  // iterate over periodic point neighbor elements
  for (auto tuple : _periodic_point_neighbors)
  {
    const auto * elem = std::get<0>(tuple);
    const auto * first = std::get<1>(tuple);
    const auto * second = std::get<2>(tuple);

    Point centroid = elem->centroid() - (*first - *second);

    const Real r_cut2 = _r_cut + elem->hmax() / 2.0;

    ret_matches.clear();
    kd_tree->radiusSearch(&(centroid(0)), r_cut2 * r_cut2, ret_matches, search_params);
    for (auto & match : ret_matches)
      _communication_lists[elem->processor_id()].insert(match.first);
  }

  // request fulfilled
  _update_communication_lists = false;
}

namespace TIMPI
{

StandardType<RadialGreensConvolution::QPData>::StandardType(
    const RadialGreensConvolution::QPData * example)
{
  // We need an example for MPI_Address to use
  static const RadialGreensConvolution::QPData p;
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

  libmesh_call_mpi(MPI_Get_address(const_cast<RadialGreensConvolution::QPData *>(example), &start));
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
  libmesh_call_mpi(
      MPI_Type_create_resized(tmptype, 0, sizeof(RadialGreensConvolution::QPData), &_datatype));
  libmesh_call_mpi(MPI_Type_free(&tmptype));

  this->commit();

#endif // LIBMESH_HAVE_MPI
}
} // namespace TIMPI

StandardType<RadialGreensConvolution::QPData>::StandardType(
    const StandardType<RadialGreensConvolution::QPData> & t)
{
#ifdef LIBMESH_HAVE_MPI
  libmesh_call_mpi(MPI_Type_dup(t._datatype, &_datatype));
#endif
}

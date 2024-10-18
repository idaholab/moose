//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WorkBalance.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"
#include "PetscExternalPartitioner.h"
#include "CastUniquePointer.h"

#include "libmesh/quadrature.h"
#include "libmesh/elem_side_builder.h"

#include <numeric>

registerMooseObject("MooseApp", WorkBalance);

InputParameters
WorkBalance::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Computes several metrics for workload balance per processor");

  // These are numbered this way because NL is always system 0 and Aux is system 1
  MooseEnum system_enum("ALL=-1 NL AUX", "ALL");
  params.addParam<MooseEnum>(
      "system",
      system_enum,
      "The system(s) to retrieve the number of DOFs from (NL, AUX, ALL). Default == ALL");

  params.addParam<bool>("sync_to_all_procs",
                        false,
                        "Whether or not to sync the vectors to all processors.  By default we only "
                        "sync them to processor 0 so they can be written out.  Setting this to "
                        "true will use more communication, but is necessary if you expect these "
                        "vectors to be available on all processors");

  MultiMooseEnum balances(
      "num_elems=0 num_nodes=1 num_dofs=2 num_partition_sides=3 partition_surface_area=4 "
      "num_partition_hardware_id_sides=5 partition_hardware_id_surface_area=6",
      "num_elems num_nodes num_dofs num_partition_sides partition_surface_area "
      "num_partition_hardware_id_sides partition_hardware_id_surface_area");
  params.addParam<MultiMooseEnum>(
      "balances", balances, "Which metrics do you want to use to represent word balance");
  return params;
}

WorkBalance::WorkBalance(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _system(getParam<MooseEnum>("system")),
    _rank_map(_app.rankMap()),
    _my_hardware_id(_rank_map.hardwareID(processor_id())),
    _sync_to_all_procs(getParam<bool>("sync_to_all_procs")),
    _local_num_elems(0),
    _local_num_nodes(0),
    _local_num_dofs(0),
    _local_num_partition_sides(0),
    _local_partition_surface_area(0),
    _local_num_partition_hardware_id_sides(0),
    _local_partition_hardware_id_surface_area(0),
    _pid(declareVector("pid")),
    _balances(getParam<MultiMooseEnum>("balances"))
{
  for (auto & balance : _balances)
    _balance_vectors[balance] = &declareVector(MooseUtils::toLower(
        balance)); // Use 'toLower' to make names consistent with the original interface
}

void
WorkBalance::initialize()
{
  _local_num_elems = 0;
  _local_num_nodes = 0;
  _local_num_dofs = 0;
  _local_num_partition_sides = 0;
  _local_partition_surface_area = 0;
  _local_num_partition_hardware_id_sides = 0;
  _local_partition_hardware_id_surface_area = 0;
}

namespace
{

// Helper Threaded Loop for Elements
class WBElementLoop : public ThreadedElementLoopBase<ConstElemRange>
{
public:
  WBElementLoop(MooseMesh & mesh, int system, const RankMap & rank_map)
    : ThreadedElementLoopBase(mesh),
      _system(system),
      _rank_map(rank_map),
      _my_hardware_id(rank_map.hardwareID(mesh.processor_id())),
      _local_num_elems(0),
      _local_num_dofs(0),
      _local_num_partition_sides(0),
      _local_partition_surface_area(0),
      _local_num_partition_hardware_id_sides(0),
      _local_partition_hardware_id_surface_area(0),
      _this_pid(_mesh.processor_id()) // Get this once because it is expensive
  {
    // This is required because dynamic_pointer_cast() requires an l-value
    auto partitioner = mesh.getMesh().partitioner()->clone();
    _petsc_partitioner = dynamic_pointer_cast<PetscExternalPartitioner>(partitioner);
  }

  WBElementLoop(WBElementLoop & x, Threads::split split)
    : ThreadedElementLoopBase(x, split),
      _system(x._system),
      _rank_map(x._rank_map),
      _my_hardware_id(x._my_hardware_id),
      _local_num_elems(0),
      _local_num_dofs(0),
      _local_num_partition_sides(0),
      _local_partition_surface_area(0),
      _local_num_partition_hardware_id_sides(0),
      _local_partition_hardware_id_surface_area(0),
      _this_pid(x._this_pid)
  {
    if (x._petsc_partitioner)
    {
      // This is required because dynamic_pointer_cast() requires an l-value
      auto partitioner = x._petsc_partitioner->clone();
      _petsc_partitioner = dynamic_pointer_cast<PetscExternalPartitioner>(partitioner);
    }
  }

  virtual ~WBElementLoop() {}

  virtual void pre() override
  {
    _local_num_elems = 0;
    _local_num_dofs = 0;
    _local_num_partition_sides = 0;
    _local_partition_surface_area = 0;
    _local_num_partition_hardware_id_sides = 0;
    _local_partition_hardware_id_surface_area = 0;
  }

  virtual void onElement(const Elem * elem) override
  {
    if (_petsc_partitioner && _petsc_partitioner->applyElementEeight())
    {
      // We should change partitioner interface to take const
      // But at this point let us keep API intact
      _local_num_elems += _petsc_partitioner->computeElementWeight(const_cast<Elem &>(*elem));
    }
    else
      _local_num_elems++;

    // Find out how many dofs there are on this element
    if (_system == WorkBalance::ALL) // All systems
    {
      auto n_sys = elem->n_systems();
      for (decltype(n_sys) sys = 0; sys < n_sys; sys++)
      {
        auto n_vars = elem->n_vars(sys);

        for (decltype(n_vars) var = 0; var < n_vars; var++)
          _local_num_dofs += elem->n_dofs(sys, var);
      }
    }
    else // Particular system
    {
      auto n_vars = elem->n_vars(static_cast<unsigned int>(_system));

      for (decltype(n_vars) var = 0; var < n_vars; var++)
        _local_num_dofs += elem->n_dofs(static_cast<unsigned int>(_system), var);
    }
  }

  virtual void onInternalSide(const Elem * elem, unsigned int side) override
  {
    if (elem->neighbor_ptr(side)->processor_id() != _this_pid)
    {
      if (_petsc_partitioner && _petsc_partitioner->applySideWeight())
      {
        // We should change partitioner interface to take const
        // But at this point let us keep API intact
        _local_num_partition_sides +=
            _petsc_partitioner->computeSideWeight(const_cast<Elem &>(*elem), side);
      }
      else
        _local_num_partition_sides++;

      // NOTE: we do not want to account for different coordinate systems here, so
      // using volume from libmesh elem is fine here
      auto volume = _elem_side_builder(*elem, side).volume();
      _local_partition_surface_area += volume;

      if (_my_hardware_id != _rank_map.hardwareID(elem->neighbor_ptr(side)->processor_id()))
      {
        _local_num_partition_hardware_id_sides++;
        _local_partition_hardware_id_surface_area += volume;
      }
    }
  }

  void join(const WBElementLoop & y)
  {
    _local_num_elems += y._local_num_elems;
    _local_num_dofs += y._local_num_dofs;
    _local_num_partition_sides += y._local_num_partition_sides;
    _local_partition_surface_area += y._local_partition_surface_area;
    _local_num_partition_hardware_id_sides += y._local_num_partition_hardware_id_sides;
    _local_partition_hardware_id_surface_area += y._local_partition_hardware_id_surface_area;
  }

  int _system;

  const RankMap & _rank_map;

  unsigned int _my_hardware_id;

  dof_id_type _local_num_elems;
  dof_id_type _local_num_dofs;
  dof_id_type _local_num_partition_sides;
  Real _local_partition_surface_area;
  dof_id_type _local_num_partition_hardware_id_sides;
  Real _local_partition_hardware_id_surface_area;

  processor_id_type _this_pid;

  libMesh::ElemSideBuilder _elem_side_builder;

  std::unique_ptr<PetscExternalPartitioner> _petsc_partitioner;

private:
  bool shouldComputeInternalSide(const Elem & /*elem*/, const Elem & /*neighbor*/) const override
  {
    return true;
  }
};

class WBNodeLoop : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  WBNodeLoop(FEProblemBase & fe_problem, int system)
    : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
      _system(system),
      _local_num_nodes(0),
      _local_num_dofs(0)
  {
  }

  WBNodeLoop(WBNodeLoop & x, Threads::split split)
    : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
      _system(x._system),
      _local_num_nodes(0),
      _local_num_dofs(0)
  {
  }

  virtual void onNode(ConstNodeRange::const_iterator & node_it)
  {
    auto & node = *(*node_it);

    _local_num_nodes++;

    // Find out how many dofs there are on this node
    if (_system == WorkBalance::ALL) // All systems
    {
      auto n_sys = node.n_systems();
      for (decltype(n_sys) sys = 0; sys < n_sys; sys++)
      {
        auto n_vars = node.n_vars(sys);

        for (decltype(n_vars) var = 0; var < n_vars; var++)
          _local_num_dofs += node.n_dofs(sys, var);
      }
    }
    else // Particular system
    {
      auto n_vars = node.n_vars(static_cast<unsigned int>(_system));

      for (decltype(n_vars) var = 0; var < n_vars; var++)
        _local_num_dofs += node.n_dofs(static_cast<unsigned int>(_system), var);
    }
  }

  void join(WBNodeLoop & y)
  {
    _local_num_nodes += y._local_num_nodes;
    _local_num_dofs += y._local_num_dofs;
  }

  int _system;

  dof_id_type _local_num_nodes;
  dof_id_type _local_num_dofs;
};

} // End of blank namespace

void
WorkBalance::execute()
{
  auto & mesh = _fe_problem.mesh();

  // Get all of the Elem info first
  WBElementLoop wb_el(mesh, _system, _rank_map);

  Threads::parallel_reduce(*mesh.getActiveLocalElementRange(), wb_el);

  _local_num_elems = wb_el._local_num_elems;
  _local_num_dofs = wb_el._local_num_dofs;
  _local_num_partition_sides = wb_el._local_num_partition_sides;
  _local_partition_surface_area = wb_el._local_partition_surface_area;
  _local_num_partition_hardware_id_sides = wb_el._local_num_partition_hardware_id_sides;
  _local_partition_hardware_id_surface_area = wb_el._local_partition_hardware_id_surface_area;

  // Now Node info
  WBNodeLoop wb_nl(_fe_problem, _system);

  Threads::parallel_reduce(*mesh.getLocalNodeRange(), wb_nl);

  _local_num_nodes = wb_nl._local_num_nodes;
  _local_num_dofs += wb_nl._local_num_dofs;
}

void
WorkBalance::gather(int balance_id, VectorPostprocessorValue & vppv)
{
  if (!_sync_to_all_procs)
  {
    switch (balance_id)
    {
      case 0: // num_elems
        _communicator.gather(0, static_cast<Real>(_local_num_elems), vppv);
        break;
      case 1: // num_nodes
        _communicator.gather(0, static_cast<Real>(_local_num_nodes), vppv);
        break;
      case 2: // num_dofs
        _communicator.gather(0, static_cast<Real>(_local_num_dofs), vppv);
        break;
      case 3: // num_partition_sides
        _communicator.gather(0, static_cast<Real>(_local_num_partition_sides), vppv);
        break;
      case 4: // partition_surface_area
        _communicator.gather(0, _local_partition_surface_area, vppv);
        break;
      case 5: // num_partition_hardware_id_sides
        _communicator.gather(0, static_cast<Real>(_local_num_partition_hardware_id_sides), vppv);
        break;
      case 6: // partition_hardware_id_surface_area
        _communicator.gather(0, _local_partition_hardware_id_surface_area, vppv);
        break;
      default:
        mooseError("Unknown balance type: ", balance_id);
    }
  }
  else
  {
    switch (balance_id)
    {
      case 0: // num_elems
        _communicator.allgather(static_cast<Real>(_local_num_elems), vppv);
        break;
      case 1: // num_nodes
        _communicator.allgather(static_cast<Real>(_local_num_nodes), vppv);
        break;
      case 2: // num_dofs
        _communicator.allgather(static_cast<Real>(_local_num_dofs), vppv);
        break;
      case 3: // num_partition_sides
        _communicator.allgather(static_cast<Real>(_local_num_partition_sides), vppv);
        break;
      case 4: // partition_surface_area
        _communicator.allgather(_local_partition_surface_area, vppv);
        break;
      case 5: // num_partition_hardware_id_sides
        _communicator.allgather(static_cast<Real>(_local_num_partition_hardware_id_sides), vppv);
        break;
      case 6: // partition_hardware_id_surface_area
        _communicator.allgather(_local_partition_hardware_id_surface_area, vppv);
        break;
      default:
        mooseError("Unknown balance type: ", balance_id);
    }
  }
}

void
WorkBalance::finalize()
{
  for (auto & balance : _balances)
  {
    auto balance_id = balance.id();

    auto & balance_vector = *_balance_vectors.at(balance);

    gather(balance_id, balance_vector);
  }
  // Fill in the PID column - this just makes plotting easier
  _pid.resize(_communicator.size());
  std::iota(_pid.begin(), _pid.end(), 0);
}

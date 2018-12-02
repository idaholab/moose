//* This file is part of the MOOSE framework
//* https://www.mooseframework.org All rights reserved, see COPYRIGHT
//* for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT Licensed
//* under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElemSideNeighborLayersVarTester.h"

#include "libmesh/parallel.h"

#include "unistd.h"

registerMooseObject("MooseTestApp", ElemSideNeighborLayersVarTester);

template <>
InputParameters
validParams<ElemSideNeighborLayersVarTester>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription(
      "Tests whether a coupled variable is correctly ghosted.  This code needs to be cleaned up");
  params.addRequiredParam<unsigned int>("rank", "The rank for which the ghosted data are recorded");

  params.registerRelationshipManagers("ElementSideNeighborLayers", "GEOMETRIC ALGEBRAIC");
  params.addParam<unsigned short>("element_side_neighbor_layers", 1, "Number of layers to ghost");
  params.addRequiredCoupledVar("u",
                               "The variable that is ghosted.  At nodes where this is NOT ghosted, "
                               "this UserObject will set the variable to -1, so to visualise the "
                               "areas where it is ghosted, initialize u to a positive value");
  return params;
}

ElemSideNeighborLayersVarTester::ElemSideNeighborLayersVarTester(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _rank(getParam<unsigned int>("rank")),
    _u_nodal(getVar("u", 0)),
    _u_var_num(coupled("u", 0))
{
}

void
ElemSideNeighborLayersVarTester::timestepSetup()
{
  _nodal_data.clear();
  auto my_processor_id = processor_id();

  if (my_processor_id == _rank)
  {
    // This loops over *all* elements for a replicated mesh and local+ghosted for distributed:
    // for (const auto & elem : _subproblem.mesh().getMesh().active_element_ptr_range())
    //
    // This loops over local + 1 layer for replicated and distributed
    for (const auto & elem : _fe_problem.getEvaluableElementRange())
    {
      Moose::err << "rank=" << my_processor_id << " elem=" << elem->id() << std::endl;
      for (unsigned i = 0; i < elem->n_nodes(); ++i)
      {
        const dof_id_type node_id = elem->node_id(i);
        const Node & node = _mesh.nodeRef(node_id);
        bool semilocal = _mesh.isSemiLocal(const_cast<Node *>(&node));

        // the following if statement should not be needed in distributed mesh,
        // while in replicated mesh i don't know what to do
        if (semilocal)
          _nodal_data[node_id] = _u_nodal->getNodalValue(node);
        else
          Moose::err << "rank=" << my_processor_id << " elem=" << elem->id()
                     << " global_node=" << node_id << " has u undefined" << std::endl;
      }
    }
  }
}

void
ElemSideNeighborLayersVarTester::finalize()
{
  // I bet the following can be done much more robustly but i'm running out of time
  unsigned size_nodal_data = _nodal_data.size();
  _communicator.broadcast(size_nodal_data, _rank);

  std::vector<dof_id_type> node_list;
  std::vector<Real> u_vals;
  if (processor_id() == _rank)
  {
    for (const auto & nd : _nodal_data)
    {
      node_list.push_back(nd.first);
      u_vals.push_back(nd.second);
    }
  }
  else
  {
    node_list.resize(size_nodal_data);
    u_vals.resize(size_nodal_data);
  }

  _communicator.broadcast(node_list, _rank);
  _communicator.broadcast(u_vals, _rank);

  for (unsigned i = 0; i < size_nodal_data; ++i)
    _nodal_data[node_list[i]] = u_vals[i];
}

Real
ElemSideNeighborLayersVarTester::getNodalValue(dof_id_type node_id) const
{
  const auto pos = _nodal_data.find(node_id);
  if (pos != _nodal_data.end())
    return pos->second;

  return -1.0;
}

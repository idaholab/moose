/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GapValueAux.h"

#include "MooseMesh.h"
#include "SystemBase.h"
#include "MooseEnum.h"

#include "string_to_enum.h"

template<>
InputParameters validParams<GapValueAux>()
{
  MooseEnum orders("FIRST, SECOND, THIRD, FOURTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary on the other side of a gap.");
  params.addRequiredParam<VariableName>("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  return params;
}

//
// Look up the MooseVariable and index directly (not using the Coupleable
// interface) to avoid an inappropriate error check.
//
MooseVariable &
getVariable(InputParameters & params, const std::string & name)
{
  SubProblem & problem = *params.get<SubProblem*>("_subproblem");
  if (!problem.hasVariable( name ))
  {
    mooseError("Unable to find variable " + name);
  }
  return problem.getVariable( params.get<THREAD_ID>("_tid"), name );
}

GapValueAux::GapValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _penetration_locator(_nodal ?  getPenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order"))) : getQuadraturePenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _moose_var(_subproblem.getVariable(_tid, getParam<VariableName>("paired_variable"))),
    _serialized_solution(_moose_var.sys().currentSolution()),
    _dof_map(_moose_var.dofMap()),
    _warnings(getParam<bool>("warnings"))
{
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  MooseVariable & pv(getVariable(parameters, getParam<VariableName>("paired_variable")));
  Order pairedVarOrder(pv.getOrder());
  Order gvaOrder(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  if (pairedVarOrder != gvaOrder && pairedVarOrder != CONSTANT)
  {
    mooseError("ERROR: specified order for GapValueAux ("<<Utility::enum_to_string<Order>(gvaOrder)
               <<") does not match order for paired_variable \""<<pv.name()<<"\" ("
               <<Utility::enum_to_string<Order>(pairedVarOrder)<<")");
  }
}

GapValueAux::~GapValueAux()
{
}

Real
GapValueAux::computeValue()
{
  const Node * current_node = NULL;

  if(_nodal)
    current_node = _current_node;
  else
    current_node = _mesh.getQuadratureNode(_current_elem, _current_side, _qp);

  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[current_node->id()];

  Real gap_value(0.0);

  if (pinfo)
  {
    std::vector<std::vector<Real> > & side_phi = pinfo->_side_phi;
    if (_moose_var.feType().order != CONSTANT)
      gap_value = _moose_var.getValue(pinfo->_side, side_phi);
    else
      gap_value = _moose_var.getValue(pinfo->_elem, side_phi);
  }
  else
  {
    if (_warnings)
    {
      std::stringstream msg;
      msg << "No gap value information found for node ";
      msg << current_node->id();
      msg << " on processor ";
      msg << libMesh::processor_id();
      mooseWarning( msg.str() );
    }
  }
  return gap_value;
}

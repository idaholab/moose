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
  MooseEnum orders("FIRST, SECOND, THIRD, FORTH", "FIRST");

  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<BoundaryName>("paired_boundary", "The boundary on the other side of a gap.");
  params.addRequiredCoupledVar("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  params.addParam<Real>("tangential_tolerance", "Tangential distance to extend edges of contact surfaces");
  params.addParam<MooseEnum>("order", orders, "The finite element order");
  params.addParam<bool>("warnings", false, "Whether to output warning messages concerning nodes not being found");
  return params;
}

GapValueAux::GapValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _penetration_locator(getPenetrationLocator(parameters.get<BoundaryName>("paired_boundary"), getParam<std::vector<BoundaryName> >("boundary")[0], Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")))),
    _serialized_solution(_nl_sys.currentSolution()),
    _dof_map(_nl_sys.dofMap()),
    _paired_variable(coupled("paired_variable")),
    _warnings(getParam<bool>("warnings"))
{
  MooseVariable & pv(*getVar("paired_variable",0));
  if (parameters.isParamValid("tangential_tolerance"))
  {
    _penetration_locator.setTangentialTolerance(getParam<Real>("tangential_tolerance"));
  }
  Order pairedVarOrder(pv.getOrder());
  Order gvaOrder(Utility::string_to_enum<Order>(parameters.get<MooseEnum>("order")));
  if (pairedVarOrder != gvaOrder)
  {
    mooseError("ERROR: specified order for GapValueAux ("<<Utility::enum_to_string<Order>(gvaOrder)
               <<") does not match order for paired_variable \""<<pv.name()<<"\" ("
               <<Utility::enum_to_string<Order>(pairedVarOrder)<<")");
  }
}

Real
GapValueAux::computeValue()
{
  PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  Real gap_temp(0.0);

  if (pinfo)
  {
    Elem * slave_side = pinfo->_side;
    std::vector<std::vector<Real> > & slave_side_phi = pinfo->_side_phi;
    std::vector<unsigned int> slave_side_dof_indices;

    _dof_map.dof_indices(slave_side, slave_side_dof_indices, _paired_variable);

    for(unsigned int i=0; i<slave_side_dof_indices.size(); ++i)
    {
      //The zero index is because we only have one point that the phis are evaluated at
      gap_temp += slave_side_phi[i][0] * (*_serialized_solution)(slave_side_dof_indices[i]);
    }
  }
  else
  {
    if (_warnings)
    {
      std::stringstream msg;
      msg << "No gap value information found for node ";
      msg << _current_node->id();
      msg << " on processor ";
      msg << libMesh::processor_id();
      mooseWarning( msg.str() );
    }
  }
  return gap_temp;
}

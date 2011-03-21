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

#include "mesh.h"

template<>
InputParameters validParams<GapValueAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<unsigned int>("paired_boundary", "The boundary on the other side of a gap.");
  params.addRequiredCoupledVar("paired_variable", "The variable to get the value of.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

GapValueAux::GapValueAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _penetration_locator(getPenetrationLocator(parameters.get<unsigned int>("paired_boundary"), getParam<std::vector<unsigned int> >("boundary")[0])),
    _paired_variable(coupled("paired_variable"))
{
  // FIXME: !!!
//  _moose_system.needSerializedSolution(true);
}

Real
GapValueAux::computeValue()
{
  Moose::PenetrationLocator::PenetrationInfo * pinfo = _penetration_locator._penetration_info[_current_node->id()];

  Elem * slave_side = pinfo->_side;
  std::vector<std::vector<Real> > & slave_side_phi = pinfo->_side_phi;
  std::vector<unsigned int> slave_side_dof_indices;
  // FIXME: !!!
//  _moose_system._dof_map->dof_indices(slave_side, slave_side_dof_indices ,_paired_variable);

  Real gap_temp = 0.0;

  // FIXME: !!!
//  for(unsigned int i=0; i<slave_side_dof_indices.size(); i++)
//    //The zero index is because we only have point that the phis are evaluated at
//    gap_temp += slave_side_phi[i][0] * _moose_system._serialized_solution(slave_side_dof_indices[i]);

  return gap_temp;
}

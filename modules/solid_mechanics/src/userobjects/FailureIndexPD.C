/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "FailureIndexPD.h"
#include "SymmTensor.h"
#include "FEProblem.h"
#include "MooseMesh.h"
#include <cmath>
#include <algorithm>
#include <set>

template<>
InputParameters validParams<FailureIndexPD>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addCoupledVar("intact_bonds","Variable to contain number of intact bonds connected to node");
  params.addCoupledVar("total_bonds","Variable to contain total number of bonds connected to node");
  return params;
}

FailureIndexPD :: FailureIndexPD(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _aux(_fe_problem.getAuxiliarySystem()),
  _IntactBondsVar(getVar("intact_bonds",0)),
  _TotalBondsVar(getVar("total_bonds",0)),
  _bond_status_old(getMaterialPropertyOld<Real>("bond_status" + getParam<std::string>("appended_property_name")))
{
}

FailureIndexPD::~FailureIndexPD()
{
}


void
FailureIndexPD::initialize()
{
  std::vector<std::string> zero_vars;
  zero_vars.push_back("intact_bonds");
  zero_vars.push_back("total_bonds");
  _aux.zeroVariables(zero_vars);
}

void
FailureIndexPD::execute()
{
  NumericVector<Number> & sln = _aux.solution();
  int nodeid0,nodeid1;
  const Node* const node0=_current_elem->get_node(0);
  const Node* const node1=_current_elem->get_node(1);

  long int ib_dof0 = node0->dof_number(_aux.number(), _IntactBondsVar->number(), 0);
  long int ib_dof1 = node1->dof_number(_aux.number(), _IntactBondsVar->number(), 0);
  long int tb_dof0 = node0->dof_number(_aux.number(), _TotalBondsVar->number(), 0);
  long int tb_dof1 = node1->dof_number(_aux.number(), _TotalBondsVar->number(), 0);
  sln.add(ib_dof0,_bond_status_old[0]);
  sln.add(ib_dof1,_bond_status_old[0]);
  sln.add(tb_dof0,1.0);
  sln.add(tb_dof1,1.0);
}


void
FailureIndexPD::threadJoin(const UserObject & u )
{
}

void
FailureIndexPD::finalize()
{
  _aux.solution().close();
}

Real
FailureIndexPD::ComputeFailureIndex(unsigned int nodeid) const
{
  NumericVector<Number> & sln = _aux.solution();
  const Node* const node = _mesh.nodePtr(nodeid);
  long int ib_dof = node->dof_number(_aux.number(), _IntactBondsVar->number(), 0);
  long int tb_dof = node->dof_number(_aux.number(), _TotalBondsVar->number(), 0);

  return 1.0 - sln(ib_dof)/sln(tb_dof);
}

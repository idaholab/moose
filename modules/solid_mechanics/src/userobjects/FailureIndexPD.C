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
  return params;
}

FailureIndexPD :: FailureIndexPD(const InputParameters & parameters) :
  ElementUserObject(parameters),
  _IntactBonds(_fe_problem.getAuxiliarySystem().addVector("intact_bonds",false,GHOSTED)),
  _TotalBonds(_fe_problem.getAuxiliarySystem().addVector("total_bonds",false,GHOSTED)),
  _bond_status_old(getMaterialPropertyOld<Real>("bond_status" + getParam<std::string>("appended_property_name")))
{
}

FailureIndexPD::~FailureIndexPD()
{
}


void
FailureIndexPD::initialize()
{
  _IntactBonds.clear();
  _TotalBonds.clear();
}

void
FailureIndexPD::execute()
{
  int nodeid0,nodeid1;
  const Node* const node0=_current_elem->get_node(0);
  const Node* const node1=_current_elem->get_node(1);

//  long int dof_number0 = node0->dof_number(_fe_problem.getAuxiliarySystem().number(), _var.number(), 0);
//  long int dof_number1 = node0->dof_number(_fe_problem.getAuxiliarySystem().number(), _var.number(), 0);
  long int dof_number0 = node0->dof_number(_fe_problem.getAuxiliarySystem().number(), 0, 0);
  long int dof_number1 = node0->dof_number(_fe_problem.getAuxiliarySystem().number(), 0, 0);
  _IntactBonds.add(dof_number0,_bond_status_old[0]);
  _IntactBonds.add(dof_number1,_bond_status_old[0]);
  _TotalBonds.add(dof_number0,1.0);
  _TotalBonds.add(dof_number1,1.0);
}


void
FailureIndexPD::threadJoin(const UserObject & u )
{
}

void
FailureIndexPD::finalize()
{
  _IntactBonds.close();
  _TotalBonds.close();
}

Real
FailureIndexPD::ComputeFailureIndex(unsigned int nodeid) const
{
  const Node* const node = _mesh.nodePtr(nodeid);
//  long int dof_number = node->dof_number(_fe_problem.getAuxiliarySystem().number(), _var.number(), 0);
  long int dof_number = node->dof_number(_fe_problem.getAuxiliarySystem().number(), 0, 0);

  return 1.0 - _IntactBonds(dof_number)/_TotalBonds(dof_number);
}

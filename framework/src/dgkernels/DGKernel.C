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

#include "DGKernel.h"
#include "Moose.h"
#include "AsmBlock.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "dof_map.h"
#include "dense_vector.h"
#include "numeric_vector.h"
#include "dense_subvector.h"
#include "libmesh_common.h"

const unsigned int DGKernel::InternalBndId = 123456789;

template<>
InputParameters validParams<DGKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params.addRequiredParam<std::string>("variable", "The name of the variable that this boundary condition applies to");
  params.addPrivateParam<unsigned int>("_boundary_id", DGKernel::InternalBndId);

  params.addPrivateParam<std::string>("built_by_action", "add_dg_kernel");
  return params;
}


DGKernel::DGKernel(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    Coupleable(parameters, false),
    TwoMaterialPropertyInterface(parameters),

    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblemInterface *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _asmb(_subproblem.asmBlock(_tid)),
    _asm_data(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<std::string>("variable"))),
    _mesh(_subproblem.mesh()),
    _dim(_mesh.dimension()),

    _current_elem(_asm_data.elem()),
    _neighbor_elem(_asm_data.neighbor()),

    _current_side(_asm_data.side()),
    _current_side_elem(_asm_data.sideElem()),

    _q_point(_subproblem.pointsFace(_tid)),
    _qrule(_subproblem.qRuleFace(_tid)),
    _JxW(_subproblem.JxWFace(_tid)),

    _boundary_id(parameters.get<unsigned int>("_boundary_id")),

    _u(_var.sln()),
    _u_old(_var.slnOld()),
    _u_older(_var.slnOlder()),

    _grad_u(_var.gradSln()),
    _grad_u_old(_var.gradSlnOld()),
    _grad_u_older(_var.gradSlnOlder()),

    _second_u(_var.secondSln()),
    _second_u_old(_var.secondSlnOld()),
    _second_u_older(_var.secondSlnOlder()),

    _phi(_asmb.phiFace()),
    _grad_phi(_asmb.gradPhiFace()),
    _second_phi(_asmb.secondPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),
    _second_test(_var.secondPhiFace()),

    _normals(_var.normals()),

    _phi_neighbor(_asmb.phiFaceNeighbor()),
    _grad_phi_neighbor(_asmb.gradPhiFaceNeighbor()),
    _second_phi_neighbor(_asmb.secondPhiFaceNeighbor()),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),
    _second_test_neighbor(_var.secondPhiFaceNeighbor()),

    _u_neighbor(_var.slnNeighbor()),
    _u_old_neighbor(_var.slnOldNeighbor()),
    _u_older_neighbor(_var.slnOlderNeighbor()),

    _grad_u_neighbor(_var.gradSlnNeighbor()),
    _grad_u_old_neighbor(_var.gradSlnOldNeighbor()),
    _grad_u_older_neighbor(_var.gradSlnOlderNeighbor()),

    _second_u_neighbor(_var.secondSlnNeighbor())
{
}

DGKernel::~DGKernel()
{
}

void
DGKernel::computeResidual()
{
  Moose::perf_log.push("computeResidual()","DGKernel");

  DenseVector<Number> & re = _asmb.residualBlock(_var.number());
  DenseVector<Number> & neighbor_re = _asmb.residualBlockNeighbor(_var.number());

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      re(_i) += _JxW[_qp]*computeQpResidual(Moose::Element);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      neighbor_re(_i) += _JxW[_qp]*computeQpResidual(Moose::Neighbor);
  }

  Moose::perf_log.pop("computeResidual()","DGKernel");
}

void
DGKernel::computeJacobian()
{
  Moose::perf_log.push("computeJacobian()","DGKernel");

  DenseMatrix<Number> & Kee = _asmb.jacobianBlock(_var.number(), _var.number());
  DenseMatrix<Number> & Ken = _asmb.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number());

  DenseMatrix<Number> & Kne = _asmb.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), _var.number());
  DenseMatrix<Number> & Knn = _asmb.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.number(), _var.number());

  for (_qp=0; _qp<_qrule->n_points(); _qp++)
  {
    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        Kee(_i,_j) += _JxW[_qp]*computeQpJacobian(Moose::ElementElement);

    for (_i=0; _i<_test.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
        Ken(_i,_j) += _JxW[_qp]*computeQpJacobian(Moose::ElementNeighbor);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi.size(); _j++)
        Kne(_i,_j) += _JxW[_qp]*computeQpJacobian(Moose::NeighborElement);

    for (_i=0; _i<_test_neighbor.size(); _i++)
      for (_j=0; _j<_phi_neighbor.size(); _j++)
        Knn(_i,_j) += _JxW[_qp]*computeQpJacobian(Moose::NeighborNeighbor);
  }

  Moose::perf_log.pop("computeJacobian()","DGKernel");
}


VariableValue &
DGKernel::coupledNeighborValue(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->slnNeighbor();
}


VariableValue &
DGKernel::coupledNeighborValueOld(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->slnOldNeighbor();
}

VariableValue &
DGKernel::coupledNeighborValueOlder(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->slnOlderNeighbor();
}

VariableGradient &
DGKernel::coupledNeighborGradient(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->gradSlnNeighbor();
}

VariableGradient &
DGKernel::coupledNeighborGradientOld(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->gradSlnOldNeighbor();
}

VariableSecond &
DGKernel::coupledNeighborSecond(const std::string & var_name, unsigned int comp)
{
  return getVar(var_name, comp)->secondSlnNeighbor();
}

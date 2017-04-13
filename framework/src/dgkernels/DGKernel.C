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
#include "Assembly.h"
#include "MooseVariable.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "MaterialData.h"
#include "ParallelUniqueId.h"

// libMesh includes
#include "libmesh/dof_map.h"
#include "libmesh/dense_vector.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/dense_subvector.h"
#include "libmesh/libmesh_common.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<DGKernel>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<TwoMaterialPropertyInterface>();
  params += validParams<TransientInterface>();
  params += validParams<BlockRestrictable>();
  params += validParams<BoundaryRestrictable>();
  params += validParams<MeshChangedInterface>();
  params.addRequiredParam<NonlinearVariableName>(
      "variable", "The name of the variable that this boundary condition applies to");
  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation. Note that in "
                        "the case this is true but no displacements "
                        "are provided in the Mesh block the "
                        "undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.declareControllable("enable");
  params.registerBase("DGKernel");

  return params;
}

DGKernel::DGKernel(const InputParameters & parameters)
  : MooseObject(parameters),
    BlockRestrictable(parameters),
    BoundaryRestrictable(parameters, false), // false for _not_ nodal
    SetupInterface(this),
    TransientInterface(this),
    FunctionInterface(this),
    UserObjectInterface(this),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(this, false, false),
    TwoMaterialPropertyInterface(this),
    Restartable(parameters, "DGKernels"),
    ZeroInterface(parameters),
    MeshChangedInterface(parameters),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _sys(*parameters.get<SystemBase *>("_sys")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _assembly(_subproblem.assembly(_tid)),
    _var(_sys.getVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _mesh(_subproblem.mesh()),

    _current_elem(_assembly.elem()),
    _current_elem_volume(_assembly.elemVolume()),

    _neighbor_elem(_assembly.neighbor()),

    _current_side(_assembly.side()),
    _current_side_elem(_assembly.sideElem()),
    _current_side_volume(_assembly.sideElemVolume()),

    _coord_sys(_assembly.coordSystem()),
    _q_point(_assembly.qPointsFace()),
    _qrule(_assembly.qRuleFace()),
    _JxW(_assembly.JxWFace()),
    _coord(_assembly.coordTransformation()),

    _u(_is_implicit ? _var.sln() : _var.slnOld()),
    _grad_u(_is_implicit ? _var.gradSln() : _var.gradSlnOld()),

    _phi(_assembly.phiFace()),
    _grad_phi(_assembly.gradPhiFace()),

    _test(_var.phiFace()),
    _grad_test(_var.gradPhiFace()),

    _normals(_var.normals()),

    _phi_neighbor(_assembly.phiFaceNeighbor()),
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor()),

    _test_neighbor(_var.phiFaceNeighbor()),
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()),

    _u_neighbor(_is_implicit ? _var.slnNeighbor() : _var.slnOldNeighbor()),
    _grad_u_neighbor(_is_implicit ? _var.gradSlnNeighbor() : _var.gradSlnOldNeighbor())
{
}

DGKernel::~DGKernel() {}

void
DGKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  DenseVector<Number> & re = is_elem ? _assembly.residualBlock(_var.number())
                                     : _assembly.residualBlockNeighbor(_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);
}

void
DGKernel::computeResidual()
{
  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
DGKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), _var.number())
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(
                      Moose::ElementNeighbor, _var.number(), _var.number())
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _var.number(), _var.number())
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _var.number(), _var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);
}

void
DGKernel::computeJacobian()
{
  // Compute element-element Jacobian
  computeElemNeighJacobian(Moose::ElementElement);

  // Compute element-neighbor Jacobian
  computeElemNeighJacobian(Moose::ElementNeighbor);

  // Compute neighbor-element Jacobian
  computeElemNeighJacobian(Moose::NeighborElement);

  // Compute neighbor-neighbor Jacobian
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

void
DGKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  const VariableTestValue & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const VariableTestValue & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx =
      type == Moose::ElementElement
          ? _assembly.jacobianBlock(_var.number(), jvar)
          : type == Moose::ElementNeighbor
                ? _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), jvar)
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), jvar)
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _var.number(), jvar);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
}

void
DGKernel::computeOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _var.number())
    computeJacobian();
  else
  {
    // Compute element-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);

    // Compute element-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);

    // Compute neighbor-element Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);

    // Compute neighbor-neighbor Jacobian
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

Real
DGKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}

MooseVariable &
DGKernel::variable()
{
  return _var;
}

SubProblem &
DGKernel::subProblem()
{
  return _subproblem;
}

const Real &
DGKernel::getNeighborElemVolume()
{
  return _assembly.neighborVolume();
}

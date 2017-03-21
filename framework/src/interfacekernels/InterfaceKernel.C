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

#include "InterfaceKernel.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<InterfaceKernel>()
{
  InputParameters params = validParams<DGKernel>();
  params.addRequiredCoupledVar("neighbor_var", "The variable on the other side of the interface.");
  params.set<std::string>("_moose_base") = "InterfaceKernel";
  return params;
}

InterfaceKernel::InterfaceKernel(const InputParameters & parameters)
  : DGKernel(parameters),
    _neighbor_var(*getVar("neighbor_var", 0)),
    _neighbor_value(_neighbor_var.slnNeighbor()),
    _grad_neighbor_value(_neighbor_var.gradSlnNeighbor())
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError(
        "In order to use an interface kernel, you must specify a boundary where it will live.");
  }
}

const MooseVariable &
InterfaceKernel::neighborVariable() const
{
  return _neighbor_var;
}

void
InterfaceKernel::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  DenseVector<Number> & re = is_elem ? _assembly.residualBlock(_var.number())
                                     : _assembly.residualBlockNeighbor(_neighbor_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      re(_i) += _JxW[_qp] * _coord[_qp] * computeQpResidual(type);
}

void
InterfaceKernel::computeElemNeighJacobian(Moose::DGJacobianType type)
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
                      Moose::ElementNeighbor, _var.number(), _neighbor_var.number())
                : type == Moose::NeighborElement
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _neighbor_var.number(), _var.number())
                      : _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor,
                                                        _neighbor_var.number(),
                                                        _neighbor_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      for (_j = 0; _j < loc_phi.size(); _j++)
        Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpJacobian(type);
}

void
InterfaceKernel::computeJacobian()
{
  computeElemNeighJacobian(Moose::ElementElement);
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

void
InterfaceKernel::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar)
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
                      ? _assembly.jacobianBlockNeighbor(
                            Moose::NeighborElement, _neighbor_var.number(), jvar)
                      : _assembly.jacobianBlockNeighbor(
                            Moose::NeighborNeighbor, _neighbor_var.number(), jvar);

  // Prevent calling of Jacobian computation if jvar doesn't lie in the current block
  if ((Kxx.m() == test_space.size()) && (Kxx.n() == loc_phi.size()))
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < test_space.size(); _i++)
        for (_j = 0; _j < loc_phi.size(); _j++)
          Kxx(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(type, jvar);
}

void
InterfaceKernel::computeElementOffDiagJacobian(unsigned int jvar)
{
  bool is_jvar_not_interface_var = true;
  if (jvar == _var.number())
  {
    computeElemNeighJacobian(Moose::ElementElement);
    is_jvar_not_interface_var = false;
  }
  if (jvar == _neighbor_var.number())
  {
    computeElemNeighJacobian(Moose::ElementNeighbor);
    is_jvar_not_interface_var = false;
  }

  if (is_jvar_not_interface_var)
  {
    computeOffDiagElemNeighJacobian(Moose::ElementElement, jvar);
    computeOffDiagElemNeighJacobian(Moose::ElementNeighbor, jvar);
  }
}

void
InterfaceKernel::computeNeighborOffDiagJacobian(unsigned int jvar)
{
  bool is_jvar_not_interface_var = true;
  if (jvar == _var.number())
  {
    computeElemNeighJacobian(Moose::NeighborElement);
    is_jvar_not_interface_var = false;
  }
  if (jvar == _neighbor_var.number())
  {
    computeElemNeighJacobian(Moose::NeighborNeighbor);
    is_jvar_not_interface_var = false;
  }

  if (is_jvar_not_interface_var)
  {
    computeOffDiagElemNeighJacobian(Moose::NeighborElement, jvar);
    computeOffDiagElemNeighJacobian(Moose::NeighborNeighbor, jvar);
  }
}

Real
InterfaceKernel::computeQpOffDiagJacobian(Moose::DGJacobianType /*type*/, unsigned int /*jvar*/)
{
  return 0.;
}

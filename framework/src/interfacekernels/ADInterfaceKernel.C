//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADInterfaceKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "ADUtils.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <typename T>
InputParameters
ADInterfaceKernelTempl<T>::validParams()
{
  InputParameters params = InterfaceKernelBase::validParams();
  if (std::is_same<T, Real>::value)
    params.registerBase("InterfaceKernel");
  else if (std::is_same<T, RealVectorValue>::value)
    params.registerBase("VectorInterfaceKernel");
  else
    ::mooseError("unsupported ADInterfaceKernelTempl specialization");
  return params;
}

template <typename T>
ADInterfaceKernelTempl<T>::ADInterfaceKernelTempl(const InputParameters & parameters)
  : InterfaceKernelBase(parameters),
    NeighborMooseVariableInterface<T>(this,
                                      false,
                                      Moose::VarKindType::VAR_NONLINEAR,
                                      std::is_same<T, Real>::value
                                          ? Moose::VarFieldType::VAR_FIELD_STANDARD
                                          : Moose::VarFieldType::VAR_FIELD_VECTOR),
    _var(*this->mooseVariable()),
    _normals(_assembly.normals()),
    _u(_var.adSln()),
    _grad_u(_var.adGradSln()),
    _ad_JxW(_assembly.adJxWFace()),
    _ad_coord(_assembly.adCoordTransformation()),
    _ad_q_point(_assembly.adQPoints()),
    _phi(_assembly.phiFace(_var)),
    _test(_var.phiFace()),
    _grad_test(_var.adGradPhiFace()),
    _neighbor_var(*getVarHelper<T>("neighbor_var", 0)),
    _neighbor_value(_neighbor_var.adSlnNeighbor()),
    _grad_neighbor_value(_neighbor_var.adGradSlnNeighbor()),
    _phi_neighbor(_assembly.phiFaceNeighbor(_neighbor_var)),
    _test_neighbor(_neighbor_var.phiFaceNeighbor()),
    _grad_test_neighbor(_neighbor_var.adGradPhiFaceNeighbor())

{
  _subproblem.haveADObjects(true);

  addMooseVariableDependency(this->mooseVariable());

  if (!parameters.isParamValid("boundary"))
    mooseError(
        "In order to use an interface kernel, you must specify a boundary where it will live.");
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const ADTemplateVariableTestValue<T> & test_space = is_elem ? _test : _test_neighbor;

  if (is_elem)
    prepareVectorTag(_assembly, _var.number());
  else
    prepareVectorTagNeighbor(_assembly, _neighbor_var.number());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
      _local_re(_i) += raw_value(_ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual(type));

  accumulateTaggedLocalResidual();
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeResidual()
{
  // in the gmsh mesh format (at least in the version 2 format) the "sideset" physical entities are
  // associated only with the lower-dimensional geometric entity that is the boundary between two
  // higher-dimensional element faces. It does not have a sidedness to it like the exodus format
  // does. Consequently we may naively try to execute an interface kernel twice, one time where _var
  // has dofs on _current_elem *AND* _neighbor_var has dofs on _neighbor_elem, and the other time
  // where _var has dofs on _neighbor_elem and _neighbor_var has dofs on _current_elem. We only want
  // to execute in the former case. In the future we should remove this and add some kind of "block"
  // awareness to interface kernels to avoid all the unnecessary reinit that happens before we hit
  // this return
  if (!_var.activeOnSubdomain(_current_elem->subdomain_id()) ||
      !_neighbor_var.activeOnSubdomain(_neighbor_elem->subdomain_id()))
    return;

  // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const ADTemplateVariableTestValue<T> & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const ADTemplateVariableTestValue<T> & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  unsigned int ivar, jvar;

  switch (type)
  {
    case Moose::ElementElement:
      ivar = jvar = _var.number();
      resid_type = Moose::Element;
      break;
    case Moose::ElementNeighbor:
      ivar = _var.number(), jvar = _neighbor_var.number();
      resid_type = Moose::Element;
      break;
    case Moose::NeighborElement:
      ivar = _neighbor_var.number(), jvar = _var.number();
      resid_type = Moose::Neighbor;
      break;
    case Moose::NeighborNeighbor:
      ivar = _neighbor_var.number(), jvar = _neighbor_var.number();
      resid_type = Moose::Neighbor;
      break;
    default:
      mooseError("Unknown DGJacobianType ", type);
  }

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, ivar, jvar);
  else
    prepareMatrixTagNeighbor(_assembly, ivar, jvar, type);

  auto ad_offset =
      Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), type, _sys.system().n_vars());

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    for (_i = 0; _i < test_space.size(); _i++)
    {
      DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual(resid_type);
      for (_j = 0; _j < loc_phi.size(); _j++)
        _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
    }

  accumulateTaggedLocalMatrix();
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeJacobian()
{
  // in the gmsh mesh format (at least in the version 2 format) the "sideset" physical entities are
  // associated only with the lower-dimensional geometric entity that is the boundary between two
  // higher-dimensional element faces. It does not have a sidedness to it like the exodus format
  // does. Consequently we may naively try to execute an interface kernel twice, one time where _var
  // has dofs on _current_elem *AND* _neighbor_var has dofs on _neighbor_elem, and the other time
  // where _var has dofs on _neighbor_elem and _neighbor_var has dofs on _current_elem. We only want
  // to execute in the former case. In the future we should remove this and add some kind of "block"
  // awareness to interface kernels to avoid all the unnecessary reinit that happens before we hit
  // this return
  if (!_var.activeOnSubdomain(_current_elem->subdomain_id()) ||
      !_neighbor_var.activeOnSubdomain(_neighbor_elem->subdomain_id()))
    return;

  computeElemNeighJacobian(Moose::ElementElement);
  computeElemNeighJacobian(Moose::NeighborNeighbor);
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                                           unsigned int jvar)
{
  const ADTemplateVariableTestValue<T> & test_space =
      (type == Moose::ElementElement || type == Moose::ElementNeighbor) ? _test : _test_neighbor;
  const ADTemplateVariableTestValue<T> & loc_phi =
      (type == Moose::ElementElement || type == Moose::NeighborElement) ? _phi : _phi_neighbor;

  unsigned int ivar;

  if (type == Moose::ElementElement || type == Moose::ElementNeighbor)
  {
    ivar = _var.number();
    resid_type = Moose::Element;
  }
  else
  {
    ivar = _neighbor_var.number();
    resid_type = Moose::Neighbor;
  }

  auto ad_offset =
      Moose::adOffset(jvar, _sys.getMaxVarNDofsPerElem(), type, _sys.system().n_vars());

  if (type == Moose::ElementElement)
    prepareMatrixTag(_assembly, ivar, jvar);
  else
    prepareMatrixTagNeighbor(_assembly, ivar, jvar, type);

  // Prevent calling of Jacobian computation if jvar doesn't lie in the current block
  if ((_local_ke.m() == test_space.size()) && (_local_ke.n() == loc_phi.size()))
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      for (_i = 0; _i < test_space.size(); _i++)
      {
        DualReal residual = _ad_JxW[_qp] * _ad_coord[_qp] * computeQpResidual(resid_type);
        for (_j = 0; _j < loc_phi.size(); _j++)
          _local_ke(_i, _j) += residual.derivatives()[ad_offset + _j];
      }

  accumulateTaggedLocalMatrix();
}

template <typename T>
void
ADInterfaceKernelTempl<T>::computeElementOffDiagJacobian(unsigned int jvar)
{
  // in the gmsh mesh format (at least in the version 2 format) the "sideset" physical entities are
  // associated only with the lower-dimensional geometric entity that is the boundary between two
  // higher-dimensional element faces. It does not have a sidedness to it like the exodus format
  // does. Consequently we may naively try to execute an interface kernel twice, one time where _var
  // has dofs on _current_elem *AND* _neighbor_var has dofs on _neighbor_elem, and the other time
  // where _var has dofs on _neighbor_elem and _neighbor_var has dofs on _current_elem. We only want
  // to execute in the former case. In the future we should remove this and add some kind of "block"
  // awareness to interface kernels to avoid all the unnecessary reinit that happens before we hit
  // this return
  if (!_var.activeOnSubdomain(_current_elem->subdomain_id()) ||
      !_neighbor_var.activeOnSubdomain(_neighbor_elem->subdomain_id()))
    return;

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

template <typename T>
void
ADInterfaceKernelTempl<T>::computeNeighborOffDiagJacobian(unsigned int jvar)
{
  // in the gmsh mesh format (at least in the version 2 format) the "sideset" physical entities are
  // associated only with the lower-dimensional geometric entity that is the boundary between two
  // higher-dimensional element faces. It does not have a sidedness to it like the exodus format
  // does. Consequently we may naively try to execute an interface kernel twice, one time where _var
  // has dofs on _current_elem *AND* _neighbor_var has dofs on _neighbor_elem, and the other time
  // where _var has dofs on _neighbor_elem and _neighbor_var has dofs on _current_elem. We only want
  // to execute in the former case. In the future we should remove this and add some kind of "block"
  // awareness to interface kernels to avoid all the unnecessary reinit that happens before we hit
  // this return
  if (!_var.activeOnSubdomain(_current_elem->subdomain_id()) ||
      !_neighbor_var.activeOnSubdomain(_neighbor_elem->subdomain_id()))
    return;

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

// Explicitly instantiates the two versions of the ADInterfaceKernelTempl class
template class ADInterfaceKernelTempl<Real>;
template class ADInterfaceKernelTempl<RealVectorValue>;

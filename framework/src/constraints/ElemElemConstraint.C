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

#include "ElemElemConstraint.h"
#include "FEProblem.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ElemElemConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<unsigned int>("interface_id", "The id of the interface.");
  return params;
}

ElemElemConstraint::ElemElemConstraint(const InputParameters & parameters) :
    Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(parameters, false, false),
    _fe_problem(*parameters.get<FEProblem *>("_fe_problem")),
    _dim(_mesh.dimension()),

    _current_elem(_assembly.elem()), 
    _current_elem_volume(_assembly.elemVolume()), 

    _neighbor_elem(_assembly.neighbor()), 
    _neighbor_elem_volume(_assembly.neighborVolume()), 

    _current_side(_assembly.side()), 
    _current_side_elem(_assembly.sideElem()), 
    _current_side_volume(_assembly.sideElemVolume()), 

    _coord_sys(_assembly.coordSystem()), 
    _q_point(_assembly.qPointsFace()), 
    _qrule(_assembly.qRuleFace()), 
    _JxW(_assembly.JxWFace()), 
    _coord(_assembly.coordTransformation()), 

    _u(_var.sln()), 
    _grad_u(_var.gradSln()), 

    _phi(_assembly.phi()), 
    _grad_phi(_assembly.gradPhi()), 

    _test(_var.phi()), 
    _grad_test(_var.gradPhi()), 

    _normals(_var.normals()), 

    _phi_neighbor(_assembly.phiFaceNeighbor()), 
    _grad_phi_neighbor(_assembly.gradPhiFaceNeighbor()), 

    _test_neighbor(_var.phiFaceNeighbor()), 
    _grad_test_neighbor(_var.gradPhiFaceNeighbor()), 

    _u_neighbor(_var.slnNeighbor()), 
    _grad_u_neighbor(_var.gradSlnNeighbor())
{
}

ElemElemConstraint::~ElemElemConstraint()
{
}

void
ElemElemConstraint::reinit(ElementPairInfo & element_pair_info)
{
  setqRuleNormal(element_pair_info);
}

void 
ElemElemConstraint::setqRuleNormal(ElementPairInfo & element_pair_info)
{
  _interface_q_point.resize(element_pair_info._q_point.size());
  _interface_JxW.resize(element_pair_info._JxW.size());
  std::copy(element_pair_info._q_point.begin(), element_pair_info._q_point.end(), _interface_q_point.begin());
  std::copy(element_pair_info._JxW.begin(), element_pair_info._JxW.end(), _interface_JxW.begin());
}

void
ElemElemConstraint::computeElemNeighResidual(Moose::DGResidualType type)
{
  bool is_elem;
  if (type == Moose::Element)
    is_elem = true;
  else
    is_elem = false;

  const VariableTestValue & test_space = is_elem ? _test : _test_neighbor;
  DenseVector<Number> & re = is_elem ? _assembly.residualBlock(_var.number()) :
                                       _assembly.residualBlockNeighbor(_var.number());
  for (_qp=0; _qp < _interface_q_point.size(); _qp++)
      for (_i=0; _i< test_space.size(); _i++)
        re(_i) += _interface_JxW[_qp] * computeQpResidual(type);
}

void
ElemElemConstraint::computeResidual()
{
 // Compute the residual for this element
  computeElemNeighResidual(Moose::Element);

  // Compute the residual for the neighbor
  computeElemNeighResidual(Moose::Neighbor);
}

void
ElemElemConstraint::computeElemNeighJacobian(Moose::DGJacobianType type)
{
  const VariableTestValue & test_space = ( type == Moose::ElementElement || type == Moose::ElementNeighbor ) ?
                                         _test : _test_neighbor;
  const VariableTestValue & loc_phi = ( type == Moose::ElementElement || type == Moose::NeighborElement ) ?
                                       _phi : _phi_neighbor;
  DenseMatrix<Number> & Kxx = type == Moose::ElementElement ? _assembly.jacobianBlock(_var.number(), _var.number()) :
                              type == Moose::ElementNeighbor ? _assembly.jacobianBlockNeighbor(Moose::ElementNeighbor, _var.number(), _var.number()) :
                              type == Moose::NeighborElement ? _assembly.jacobianBlockNeighbor(Moose::NeighborElement, _var.number(), _var.number()) :
                              _assembly.jacobianBlockNeighbor(Moose::NeighborNeighbor, _var.number(), _var.number());

  for (_qp=0; _qp< _interface_q_point.size(); _qp++)
    for (_i=0; _i<test_space.size(); _i++)
      for (_j=0; _j<loc_phi.size(); _j++)
        Kxx(_i,_j) += _interface_JxW[_qp]*computeQpJacobian(type);
}

void
ElemElemConstraint::computeJacobian()
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

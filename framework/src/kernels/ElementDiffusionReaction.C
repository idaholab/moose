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

#include "ElementDiffusionReaction.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "Problem.h"
#include "SubProblem.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<ElementDiffusionReaction>()
{
  InputParameters params = validParams<KernelBase>();
  params += validParams<ElementInterface>();
  params.addParam<Real>("diffusion_coefficient", 1.0, "Diffusion coefficient");
  params.addParam<Real>("reaction_coefficient", 1.0, "Reaction coefficient");
  params.addParam<Real>("source", 1.0, "Reaction coefficient");
  params.registerBase("Kernel");
  return params;
}

ElementDiffusionReaction::ElementDiffusionReaction(const InputParameters & parameters)
  : KernelBase(parameters),
    ElementInterface(this),
    _u_coef(_var.nodalSln()),
    _d(getParam<Real>("diffusion_coefficient")),
    _r(getParam<Real>("reaction_coefficient")),
    _src(3, getParam<Real>("source"))
{
}

void
ElementDiffusionReaction::computeResidual()
{
  DenseVector<Number> & re = _ei_assembly.residualBlock(_var.number());
  massProduct(_u_coef, _r, re);
  massProduct(_src, _r, re);
  stiffnessProduct(_u_coef, _d, re);
}

void
ElementDiffusionReaction::computeJacobian()
{
  DenseMatrix<Number> & ke = _ei_assembly.jacobianBlock(_var.number(), _var.number());
  massMatrix(_r, ke);
  stiffnessMatrix(_d, ke);
}

void
ElementDiffusionReaction::computeOffDiagJacobian(unsigned int /*jvar*/)
{
}

void
ElementDiffusionReaction::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

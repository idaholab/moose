//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSADDisplaceBoundaryBC.h"
#include "SystemBase.h"
#include "ImplicitEuler.h"

registerMooseObject("NavierStokesApp", INSADDisplaceBoundaryBC);

InputParameters
INSADDisplaceBoundaryBC::validParams()
{
  InputParameters params = ADNodalBC::validParams();
  params.addClassDescription("Boundary condition for displacing a boundary");
  params.addRequiredParam<MooseFunctorName>("velocity", "The velocity at which to displace");
  params.addRequiredParam<unsigned short>(
      "component", "What component of velocity/displacement this object is acting on.");
  params.addRequiredParam<SubdomainName>(
      "associated_subdomain",
      "The subdomain that the boundary nodeset is associated with. This will be passed to the "
      "coupled functor for unambigious evaluation (e.g. at the edge of the node-patch where we "
      "might run into the intersection of subdomains");
  return params;
}

INSADDisplaceBoundaryBC::INSADDisplaceBoundaryBC(const InputParameters & parameters)
  : ADNodalBC(parameters),
    _velocity(getFunctor<ADRealVectorValue>("velocity")),
    _u_old(_var.nodalValueOld()),
    _component(getParam<unsigned short>("component")),
    _sub_id(_mesh.getSubdomainID(getParam<SubdomainName>("associated_subdomain")))
{
  if (!dynamic_cast<const ImplicitEuler *>(&_sys.getTimeIntegrator(_var.number())))
    mooseError("This boundary condition hard-codes a displacement update with the form of an "
               "implicit Euler discretization. Consequently please use the default time "
               "integrator, ImplicitEuler.");
}

ADReal
INSADDisplaceBoundaryBC::computeQpResidual()
{
  const Moose::NodeArg nd{_current_node, _sub_id};
  return _u - (_u_old + this->_dt * _velocity(nd, determineState())(_component));
}

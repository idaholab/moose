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

#include "DarcyVelocity.h"

template<>
InputParameters validParams<DarcyVelocity>()
{
  InputParameters params = validParams<AuxKernel>();

  // Declare the options for a MooseEnum.
  // These options will be presented to the user in Peacock
  // and if something other than these options is in the input file
  // an error will be printed
  MooseEnum component("x y z");

  // Use the MooseEnum to add a parameter called "component"
  params.addRequiredParam<MooseEnum>("component", component, "The desired component of velocity.");

  // Add a "coupling paramater" to get a variable from the input file.
  params.addRequiredCoupledVar("darcy_pressure", "The pressure field.");

  return params;
}

DarcyVelocity::DarcyVelocity(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),

    // This will automatically convert the MooseEnum to an integer
    _component(getParam<MooseEnum>("component")),

    // Get the gradient of the variable
    _pressure_gradient(coupledGradient("darcy_pressure")),

    // Snag permeability from the Material system.
    // Only AuxKernels operating on Elemental Auxiliary Variables can do this
    _permeability(getMaterialProperty<Real>("permeability")),

    // Snag viscosity from the Material system.
    // Only AuxKernels operating on Elemental Auxiliary Variables can do this
    _viscosity(getMaterialProperty<Real>("viscosity"))
{
}

Real
DarcyVelocity::computeValue()
{
  // Access the gradient of the pressure at this quadrature point
  // Then pull out the "component" of it we are looking for (x, y or z)
  // Note that getting a particular component of a gradient is done using the
  // parenthesis operator
  return -(_permeability[_qp]/_viscosity[_qp])*_pressure_gradient[_qp](_component);
}

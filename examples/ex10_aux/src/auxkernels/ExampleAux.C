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

#include "ExampleAux.h"

template<>
InputParameters validParams<ExampleAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("value", 0.0, "Scalar value used for our auxiliary calculation");
  params.addRequiredCoupledVar("coupled", "Coupled variable");
  return params;
}

ExampleAux::ExampleAux(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   
   // We can couple in a value from one of our kernels with a call to coupledValueAux 
   _coupled_val(coupledValue("coupled")),

   // Set our member scalar value from InputParameters (read from the input file)
   _value(getParam<Real>("value"))
{}

/**
 * Auxiliary Kernels override computeValue() instead of computeQpResidual().  Aux Variables
 * are calculated either one per elemenet or one per node depending on whether we declare
 * them as "Elemental (Constant Monomial)" or "Nodal (First Lagrange)".  No changes to the
 * source are necessary to switch from one type or the other.
 */
Real
ExampleAux::computeValue()
{
  return _coupled_val[_qp] + _value;
}

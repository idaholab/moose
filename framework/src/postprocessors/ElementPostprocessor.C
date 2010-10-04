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

#include "Moose.h"
#include "ElementPostprocessor.h"

template<>
InputParameters validParams<ElementPostprocessor>()
{
  InputParameters params = validParams<Kernel>();
  params += validParams<Postprocessor>();
  params.addParam<subdomain_id_type>("block", Moose::ANY_BLOCK_ID, "block ID where the postprocessor works");
  return params;
}

ElementPostprocessor::ElementPostprocessor(const std::string & name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   Postprocessor(name, moose_system, parameters),
   _block_id(getParam<subdomain_id_type>("block"))
{
}

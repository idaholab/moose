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

#include "MaterialStateVectorAux.h"

template<>
InputParameters validParams<MaterialStateVectorAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("vpStatefulProperty", "The vector material property name");
  params.addRequiredParam<int>("index", "The index of a vector material"); 
  return params;
}

MaterialStateVectorAux::MaterialStateVectorAux(const std::string & name, 
                                               InputParameters parameters)
  :AuxKernel( name, parameters ),
   _vp_stateful_property( getMaterialProperty<std::vector<double> >("vpStatefulProperty") ),
   _index( getParam<int>("index") )
{}

double
MaterialStateVectorAux::computeValue()
{
  return _vp_stateful_property[_qp][_index];
}

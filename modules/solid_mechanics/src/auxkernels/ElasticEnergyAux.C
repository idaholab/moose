/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ElasticEnergyAux.h"

#include "SymmTensor.h"

template<>
InputParameters validParams<ElasticEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

ElasticEnergyAux::ElasticEnergyAux(const std::string & name,
                                   InputParameters parameters) :
    AuxKernel( name, parameters ),
    _stress( getMaterialProperty<SymmTensor>("stress") ),
    _elastic_strain(getMaterialProperty<SymmTensor>("elastic_strain"))
{}

Real
ElasticEnergyAux::computeValue()
{
  return 0.5*_stress[_qp].doubleContraction(_elastic_strain[_qp]);
}

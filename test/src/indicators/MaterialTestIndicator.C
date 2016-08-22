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

#include "MaterialTestIndicator.h"
#include "Assembly.h"

template<>
InputParameters validParams<MaterialTestIndicator>()
{
  InputParameters params = validParams<Indicator>();
  params += validParams<MaterialPropertyInterface>();
  params.addParam<MaterialPropertyName>("property", "The name of the material property to use for an indicator.");
  return params;
}


MaterialTestIndicator::MaterialTestIndicator(const InputParameters & parameters) :
    Indicator(parameters),
    MaterialPropertyInterface(this),
    _property(getMaterialProperty<Real>("property")),
    _qrule(_assembly.qRule()),
    _indicator_var(_sys.getVariable(_tid, name()))
{
}


void
MaterialTestIndicator::computeIndicator()
{
  std::vector<Real> min(_qrule->n_points());
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    min[qp] = _property[qp];
  _indicator_var.setNodalValue(*std::min_element(min.begin(), min.end()));
}

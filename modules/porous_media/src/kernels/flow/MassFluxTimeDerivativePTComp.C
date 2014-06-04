/****************************************************************/
/*             DO NOT MODIFY OR REMOVE THIS HEADER              */
/*          FALCON - Fracturing And Liquid CONvection           */
/*                                                              */
/*       (c) pending 2012 Battelle Energy Alliance, LLC         */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "MassFluxTimeDerivativePTComp.h"
#include "Material.h"

template<>
InputParameters validParams<MassFluxTimeDerivativePTComp>()
{
  InputParameters params = validParams<TimeDerivative>();
    params.addParam<bool>("has_chem_reactions", false, "flag if chemical reactions are present");
    return params;
}

MassFluxTimeDerivativePTComp::MassFluxTimeDerivativePTComp(const std::string & name, InputParameters parameters)
  :TimeDerivative(name, parameters),
   _density_water(getMaterialProperty<Real>("density_water")),
   _density_water_old(getMaterialProperty<Real>("time_old_density_water")),
   _dwdp(getMaterialProperty<Real>("dwdp")),

   _has_chem_reactions(getParam<bool>("has_chem_reactions")),
   _porosity(getMaterialProperty<Real>("porosity")),
   _compressibility(getMaterialProperty<Real>("compressibility")),
   _porosity_old(_has_chem_reactions ? &getMaterialPropertyOld<Real>("porosity") : &getMaterialProperty<Real>("porosity")),
   _u_old(valueOld())
{}

Real
MassFluxTimeDerivativePTComp::computeQpResidual()
{
  return _porosity[_qp]*_density_water[_qp]*_compressibility[_qp]*TimeDerivative::computeQpResidual();
}

Real
MassFluxTimeDerivativePTComp::computeQpJacobian()
{
  return _porosity[_qp]*_density_water[_qp]*_compressibility[_qp] * TimeDerivative::computeQpJacobian();
}

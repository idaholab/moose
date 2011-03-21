#include "ElasticEnergyAux.h"


template<>
InputParameters validParams<ElasticEnergyAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  return params;
}

ElasticEnergyAux::ElasticEnergyAux( const std::string & name,
                      InputParameters parameters )
  :AuxKernel( name, parameters ),
   _stress( getMaterialProperty<RealTensorValue>("stress") ),
   _grad_disp_x(coupledGradient("disp_x")),
   _grad_disp_y(_dim > 1 ? coupledGradient("disp_y") : _grad_zero),
   _grad_disp_z(_dim > 2 ? coupledGradient("disp_z") : _grad_zero)
{
 
}

Real
ElasticEnergyAux::computeValue()
{
  ColumnMajorMatrix strain(_grad_disp_x[_qp],_grad_disp_y[_qp],_grad_disp_z[_qp]);
  ColumnMajorMatrix comp_stress(_stress[_qp]);
  
  return 0.5*comp_stress.doubleContraction(strain);
}



#include "StressOutput.h"

#include "Material.h"

template<>
InputParameters validParams<StressOutput>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<bool>("VonMises",false,"Output Von Mises stress?, true or false");
  params.addParam<bool>("Hydrostatic",false,"Output hydrostatic stress?, true or false");
  params.addParam<int>("comp1", 0,"first component of stress, if not Von Mises");
  params.addParam<int>("comp2", 0,"second component of stress, if not Von Mises");
  
  return params;
}


StressOutput::StressOutput(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Kernel(name, moose_system, parameters),
   _stress(getMaterialProperty<RealTensorValue>("stress")),
   _VonMises(getParam<bool>("VonMises")),
   _Hydrostatic(getParam<bool>("Hydrostatic")),
   _comp1(getParam<int>("comp1")),
   _comp2(getParam<int>("comp2"))
{
}

Real
StressOutput::computeQpResidual()
{
  Real CalcStress = 0.0;
  if (_VonMises)
  {
    CalcStress = std::sqrt( (std::pow(_stress[_qp](0,0) - _stress[_qp](1,1), 2.0) + std::pow(_stress[_qp](1,1) - _stress[_qp](2,2),2.0) + std::pow(_stress[_qp](2,2) - _stress[_qp](0,0),2.0))/2.0);
    CalcStress += std::sqrt(3.0*(_stress[_qp](0,1)*_stress[_qp](0,1)+_stress[_qp](0,1)*_stress[_qp](0,2) + _stress[_qp](2,3)*_stress[_qp](1,2)));
  }
  else if (_Hydrostatic)
  {
    CalcStress = 1.0/3.0*(_stress[_qp](0,0) + _stress[_qp](1,1) + _stress[_qp](2,2));
  }
  else
  {  
    CalcStress = _stress[_qp](_comp1,_comp2);
  }
  
  return (_u[_qp] - CalcStress) * _test[_i][_qp];
}

Real
StressOutput::computeQpJacobian()
{
  return _phi[_j][_qp] * _test[_i][_qp];
}

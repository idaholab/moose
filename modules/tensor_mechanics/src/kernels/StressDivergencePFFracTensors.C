#include "StressDivergencePFFracTensors.h"
#include "Material.h"

template<>
InputParameters validParams<StressDivergencePFFracTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addCoupledVar("c", "Phase field damage variable: Used to indicate calculation of Off Diagonal Jacobian term");
  params.addParam<std::string>("pff_jac_prop_name","","Name of property variable containing d_stress_d_c");
  params.addClassDescription("Stress divergence kernel for phase-field fracture that additionally computes off diagonal damage dependent Jacobian components");

  return params;
}


StressDivergencePFFracTensors::StressDivergencePFFracTensors(const std::string & name, InputParameters parameters) :
  StressDivergenceTensors(name, parameters),
  _pff_jac_prop_name(getParam<std::string>("pff_jac_prop_name")),
  _c_coupled(isCoupled("c")),
  _c_var(_c_coupled ? coupled("c") : 0)
{
  if ( _c_coupled )
  {
    if ( _pff_jac_prop_name == "" )
      mooseError("StressDivergencePFFracTensors: Provide pff_jac_prop_name that contains d_stress_d_c: Coupled variable only used in Jacobian evaluation");
    else
      //Material property storing d_stress_dc
      _d_stress_dc  = &getMaterialProperty<RankTwoTensor>(_pff_jac_prop_name);
  }
}

Real
StressDivergencePFFracTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real val = StressDivergenceTensors::computeQpOffDiagJacobian( jvar );

  if ( _c_coupled && jvar == _c_var )
  {
    val = 0.0;
    for (unsigned int k = 0;k < 3; ++k)
      val += (*_d_stress_dc)[_qp](_component,k) * _grad_test[_i][_qp](k);
    val *= _phi[_j][_qp];

    return val;
  }

  //Returns if coupled variable is not c (damage variable)
  return val;
}

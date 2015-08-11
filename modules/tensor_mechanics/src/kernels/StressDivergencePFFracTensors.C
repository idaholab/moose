#include "StressDivergencePFFracTensors.h"
#include "Material.h"

template<>
InputParameters validParams<StressDivergencePFFracTensors>()
{
  InputParameters params = validParams<StressDivergenceTensors>();
  params.addClassDescription("Stress divergence kernel for phase-field fracture: Additionally computes off diagonal damage dependent Jacobian components");
  params.addCoupledVar("c", "Phase field damage variable: Used to indicate calculation of Off Diagonal Jacobian term");
  params.addParam<MaterialPropertyName>("pff_jac_prop_name", "Name of property variable containing d_stress_d_c");
  return params;
}


StressDivergencePFFracTensors::StressDivergencePFFracTensors(const InputParameters & parameters) :
    StressDivergenceTensors(parameters),
    _c_coupled(isCoupled("c")),
    _c_var(_c_coupled ? coupled("c") : 0)
{
  if (_c_coupled)
  {
    if (!isParamValid("pff_jac_prop_name"))
      mooseError("StressDivergencePFFracTensors: Provide pff_jac_prop_name that contains d_stress_d_c: Coupled variable only used in Jacobian evaluation");
    else
      _d_stress_dc  = &getMaterialProperty<RankTwoTensor>("pff_jac_prop_name");
  }
}

Real
StressDivergencePFFracTensors::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_c_coupled && jvar == _c_var)
  {
    Real val = 0.0;
    for (unsigned int k = 0;k < 3; ++k)
      val += (*_d_stress_dc)[_qp](_component,k) * _grad_test[_i][_qp](k);
    return val * _phi[_j][_qp];
  }

  //Returns if coupled variable is not c (damage variable)
  return StressDivergenceTensors::computeQpOffDiagJacobian(jvar);
}


#include "AppliedStressDivergence.h"

#include "Material.h"
#include "ElasticityTensorR4.h"

template<>
InputParameters validParams<AppliedStressDivergence>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addParam<std::string>("appended_property_name", "", "Name appended to material properties to make them unique");
  params.addRequiredParam<std::vector<Real> >("applied_strain_vector","Applied strain: e11, e22, e33, e23, e13, e12");

  params.set<bool>("use_displaced_mesh") = false;
  
  return params;
}


AppliedStressDivergence::AppliedStressDivergence(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),
   _elasticity_tensor(getMaterialProperty<ElasticityTensorR4>("elasticity_tensor" + getParam<std::string>("appended_property_name"))),
   _applied_strain_vector(getParam<std::vector<Real> >("applied_strain_vector")),
   _component(getParam<unsigned int>("component"))
{
  //Initialize applied strain tensor from input vector
  _applied_strain.fillFromInputVector(_applied_strain_vector);
  //_applied_strain.print();
}

Real
AppliedStressDivergence::computeQpResidual()
{
  //Stress divergence for applied strain; value is negative from equation
  RankTwoTensor stress;
  stress = -_elasticity_tensor[_qp]*_applied_strain;
  
  return stress.row(_component)*_grad_test[_i][_qp];
}

Real
AppliedStressDivergence::computeQpJacobian()
{
  return 0.0;
}

#include "HomogenizationKernel.h"
#include "Material.h"
#include "SymmElasticityTensor.h"

template<>
InputParameters validParams<HomogenizationKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredParam<unsigned int>("column", "An integer corresponding to the direction the variable this kernel acts in. (0 for xx, 1 for yy, 2 for zz, 3 for xy, 4 for yz, 5 for zx)");

  return params;
}


HomogenizationKernel::HomogenizationKernel(const std::string & name, InputParameters parameters)
  :Kernel(name, parameters),

   _elasticity_tensor(getMaterialProperty<SymmElasticityTensor>("elasticity_tensor")),
   _component(getParam<unsigned int>("component")),
   _column(getParam<unsigned int>("column"))
{}

Real
HomogenizationKernel::computeQpResidual()
{
  int k, l;
  int I,J;
  

  if(_column == 0)
  {
    k = 0;
    l = 0;
  }

  
  if(_column == 1)
  {
    k = 1;
    l = 1;
  }

  
  if(_column == 2)
  {
    k = 2;
    l = 2;
  }

  
  if(_column == 3)
  {
    k = 0;
    l = 1;
  }

  
  if(_column == 4)
  {
    k = 1;
    l = 2;
  }

  if(_column == 5)
  {
    k = 2;
    l = 0;
  }


  J = 3 * l + k;
  
    
  ColumnMajorMatrix E(9,9);
  Real value;

  value = 0.0;
     
  E = _elasticity_tensor[_qp].columnMajorMatrix9x9();

  for(int j = 0; j < 3; j++)
  {
    I = 3 * j + _component;
    value = value - E(I,J) * _grad_test[_i][_qp](j);
  }

  return value;
  
  
}


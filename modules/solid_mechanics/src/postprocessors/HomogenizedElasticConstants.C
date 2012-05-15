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

#include "HomogenizedElasticConstants.h"
#include "SymmElasticityTensor.h"

template<>
InputParameters validParams<HomogenizedElasticConstants>()
{
  InputParameters params = validParams<ElementIntegral>();
  params.addRequiredParam<unsigned int>("column", "An integer corresponding to the direction the variable this kernel acts in. (0 for xx, 1 for yy, 2 for zz, 3 for xy, 4 for yz, 5 for zx)");
  params.addRequiredParam<unsigned int>("row", "An integer corresponding to the direction the variable this kernel acts in. (0 for xx, 1 for yy, 2 for zz, 3 for xy, 4 for yz, 5 for zx)");
  return params;
}

HomogenizedElasticConstants::HomogenizedElasticConstants(const std::string & name, InputParameters parameters)
  :ElementIntegral(name, parameters),
   _grad_disp_x_xx(coupledGradient("dx_xx")),
   _grad_disp_y_xx(coupledGradient("dy_xx")),
   _grad_disp_z_xx(coupledGradient("dz_xx")),
   _grad_disp_x_yy(coupledGradient("dx_yy")),
   _grad_disp_y_yy(coupledGradient("dy_yy")),
   _grad_disp_z_yy(coupledGradient("dz_yy")),
   _grad_disp_x_zz(coupledGradient("dx_zz")),
   _grad_disp_y_zz(coupledGradient("dy_zz")),
   _grad_disp_z_zz(coupledGradient("dz_zz")),
   _grad_disp_x_xy(coupledGradient("dx_xy")),
   _grad_disp_y_xy(coupledGradient("dy_xy")),
   _grad_disp_z_xy(coupledGradient("dz_xy")),
   _grad_disp_x_yz(coupledGradient("dx_yz")),
   _grad_disp_y_yz(coupledGradient("dy_yz")),
   _grad_disp_z_yz(coupledGradient("dz_yz")),
   _grad_disp_x_zx(coupledGradient("dx_zx")),
   _grad_disp_y_zx(coupledGradient("dy_zx")),
   _grad_disp_z_zx(coupledGradient("dz_zx")),
   _elasticity_tensor(getMaterialProperty<SymmElasticityTensor>("elasticity_tensor" + getParam<std::string>("appended_property_name"))),
   _column(getParam<unsigned int>("column")),
   _row(getParam<unsigned int>("row")),
   _integral_value(0)
{


  if(_column == 0)
  {
    _k = 0;
    _l = 0;
  }


  if(_column == 1)
  {
    _k = 1;
    _l = 1;
  }


  if(_column == 2)
  {
    _k = 2;
    _l = 2;
  }


  if(_column == 3)
  {
    _k = 0;
    _l = 1;
  }


  if(_column == 4)
  {
    _k = 1;
    _l = 2;
  }

  if(_column == 5)
  {
    _k = 2;
    _l = 0;
  }





  if(_row == 0)
  {
    _i = 0;
    _j = 0;
  }


  if(_row == 1)
  {
    _i = 1;
    _j = 1;
  }


  if(_row == 2)
  {
    _i = 2;
    _j = 2;
  }


  if(_row == 3)
  {
    _i = 0;
    _j = 1;
  }


  if(_row == 4)
  {
    _i = 1;
    _j = 2;
  }

  if(_row == 5)
  {
    _i = 2;
    _j = 0;
  }



  _J = (3 * _l + _k);
  _I = (3 * _j + _i);

}

void
HomogenizedElasticConstants::initialize()
{
  _integral_value = 0;
}

void
HomogenizedElasticConstants::execute()
{
  _integral_value += computeIntegral();
}

Real
HomogenizedElasticConstants::getValue()
{

  gatherSum(_integral_value);

  return _integral_value;
}



void
HomogenizedElasticConstants::threadJoin(const Postprocessor & y)
{
  const HomogenizedElasticConstants & pps = dynamic_cast<const HomogenizedElasticConstants &>(y);

  _integral_value += pps._integral_value;
}

Real
HomogenizedElasticConstants::computeQpIntegral()
{
    ColumnMajorMatrix E( _elasticity_tensor[_qp].columnMajorMatrix9x9() );
    Real value;

    value = 0.0;

    VariableGradient * grad[6][3];
    grad[0][0] = &_grad_disp_x_xx;
    grad[0][1] = &_grad_disp_y_xx;
    grad[0][2] = &_grad_disp_z_xx;

    grad[1][0] = &_grad_disp_x_yy;
    grad[1][1] = &_grad_disp_y_yy;
    grad[1][2] = &_grad_disp_z_yy;

    grad[2][0] = &_grad_disp_x_zz;
    grad[2][1] = &_grad_disp_y_zz;
    grad[2][2] = &_grad_disp_z_zz;

    grad[3][0] = &_grad_disp_x_xy;
    grad[3][1] = &_grad_disp_y_xy;
    grad[3][2] = &_grad_disp_z_xy;

    grad[4][0] = &_grad_disp_x_yz;
    grad[4][1] = &_grad_disp_y_yz;
    grad[4][2] = &_grad_disp_z_yz;

    grad[5][0] = &_grad_disp_x_zx;
    grad[5][1] = &_grad_disp_y_zx;
    grad[5][2] = &_grad_disp_z_zx;


    for(int p = 0; p < 3; p++)
    {
      for(int q = 0; q < 3; q++)
      {
        value = value + E(_I,3 * q + p) * (*grad[_column][p])[_qp](q);
      }
    }

    return (E(_I,_J) - value);
}

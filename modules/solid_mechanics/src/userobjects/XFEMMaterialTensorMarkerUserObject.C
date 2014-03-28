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

#include "XFEMMaterialTensorMarkerUserObject.h"

template<>
InputParameters validParams<XFEMMaterialTensorMarkerUserObject>()
{
  InputParameters params = validParams<XFEMMarkerUserObject>();
  params += validParams<MaterialTensorCalculator>();
  params.addRequiredParam<std::string>("tensor", "The material tensor name.");
  params.addRequiredParam<Real>("threshold", "The threshold for crack growth.");
  params.addRequiredParam<bool>("average", "Should the tensor quantity be averaged over the quadruature points?");
  return params;
}

XFEMMaterialTensorMarkerUserObject::XFEMMaterialTensorMarkerUserObject(const std::string & name, InputParameters parameters):
  XFEMMarkerUserObject(name, parameters),
  _material_tensor_calculator(name, parameters),
  _tensor(getMaterialProperty<SymmTensor>(getParam<std::string>("tensor"))),
  _threshold(getParam<Real>("threshold")),
  _average(getParam<bool>("average"))
{
}

bool
XFEMMaterialTensorMarkerUserObject::doesElementCrack(RealVectorValue &direction)
{
  bool does_it_crack = false;
  unsigned int numqp = _qrule->n_points();
  std::vector<Real> tensor_quantities;
  tensor_quantities.reserve(numqp);
  std::vector<RealVectorValue> directions;
  directions.resize(numqp);
  Real ave_quantity = 0;
  Real max_quantity = 0;
  unsigned int max_index = 999999;

  for ( unsigned int qp = 0; qp < numqp; ++qp )
  {
    tensor_quantities[qp] = _material_tensor_calculator.getTensorQuantity(_tensor[qp],&_q_point[qp],directions[qp]);
    if (directions[qp](0) == 0 &&
        directions[qp](1) == 0 &&
        directions[qp](2) == 0)
    {
      mooseError("Direction has zero length in XFEMMaterialTensorMarkerUserObject");
    }
    ave_quantity += tensor_quantities[qp];
    if (tensor_quantities[qp] > max_quantity)
    {
      max_quantity = tensor_quantities[qp];
      max_index = qp;
    }
  }

  if (_average)
  {
    ave_quantity /= (Real)numqp;
    if (ave_quantity > _threshold)
    {
      does_it_crack = true;
      direction.zero();
      for ( unsigned int qp = 0; qp < numqp; ++qp )
      {
        //direction += tensor_quantities[qp] * directions[qp];
        direction += directions[qp];
      }
      direction /= direction.size();
    }
  }
  else
  {
    if (max_quantity > _threshold)
    {
      does_it_crack = true;
      direction = tensor_quantities[max_index];
    }
  }

  return does_it_crack;
}

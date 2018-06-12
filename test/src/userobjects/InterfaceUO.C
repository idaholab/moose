//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUO.h"

registerMooseObject("MooseTestApp", InterfaceUO);

template <>
InputParameters
validParams<InterfaceUO>()
{
  InputParameters params = validParams<InterfaceUserObject>();
  params.addParam<MaterialPropertyName>("mpName",
                                        "diffusivity",
                                        "The name of the material property that "
                                        "grabeed on both side of the interface");
  params.addParam<bool>(
      "use_old_prop",
      false,
      "A Boolean to indicate whether the current or old value of a material prop should be used.");
  params.addRequiredCoupledVar("variable", "the variable name");
  return params;
}

InterfaceUO::InterfaceUO(const InputParameters & parameters)
  : InterfaceUserObject(parameters),
    _u(coupledValue("variable")),
    _u_neighbor(coupledNeighborValue("variable")),
    _mean_mat_prop(0.),
    _mean_var_jump(0.),
    _total_volume(0.),
    _diffusivity_prop(getParam<bool>("use_old_prop") ? getMaterialPropertyOld<Real>("mpName")
                                                     : getMaterialProperty<Real>("mpName")),
    _neighbor_diffusivity_prop(getParam<bool>("use_old_prop")
                                   ? getNeighborMaterialPropertyOld<Real>("mpName")
                                   : getNeighborMaterialProperty<Real>("mpName"))
{
}

InterfaceUO::~InterfaceUO() {}

void
InterfaceUO::initialize()
{
  _mean_mat_prop = 0;
  _mean_var_jump = 0;
  _total_volume = 0;
}

void
InterfaceUO::execute()
{
  std::cout << "InterfaceUO::execute" << std::endl;
  for (unsigned int qp = 0; qp < _q_point.size(); ++qp)
  {
    _total_volume += (_current_side_volume + getNeighborElemVolume()) / 2;
    std::cout << "  _diffusivity_prop[qp] " << _diffusivity_prop[qp] << std::endl;
    std::cout << "  _neighbor_diffusivity_prop[qp] " << _neighbor_diffusivity_prop[qp] << std::endl;
    _mean_mat_prop += (_diffusivity_prop[qp] + _neighbor_diffusivity_prop[qp]) / 2;
    std::cout << "  _u[qp] " << _u[qp] << std::endl;
    std::cout << "  _u_neighbor[qp] " << _u_neighbor[qp] << std::endl;
    _mean_var_jump += (_u[qp] - _u_neighbor[qp]);
  }
}

void
InterfaceUO::finalize()
{
  std::cout << "InterfaceUO::finalize" << std::endl;
  gatherSum(_total_volume);
  std::cout << "_total_volume " << _total_volume << std::endl;

  gatherSum(_mean_mat_prop);
  _mean_mat_prop = _mean_mat_prop / _total_volume;
  std::cout << "_mean_mat_prop " << _mean_mat_prop << std::endl;

  gatherSum(_mean_var_jump);
  _mean_var_jump = _mean_var_jump / _total_volume;
  std::cout << "_mean_var_jump " << _mean_var_jump << std::endl;
}

void
InterfaceUO::threadJoin(const UserObject & uo)
{
  std::cout << "InterfaceUO::threadJoin" << std::endl;
  const InterfaceUO & u = dynamic_cast<const InterfaceUO &>(uo);
  _mean_mat_prop += u._mean_mat_prop;
  _mean_var_jump += u._mean_var_jump;
  _total_volume += u._total_volume;
}

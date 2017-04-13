/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "StressUpdateBase.h"

#include "MooseMesh.h"

template <>
InputParameters
validParams<StressUpdateBase>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculates the effective inelastic strain increment required to "
                             "return the isotropic stress state to a J2 yield surface.  This class "
                             "is intended to be a parent class for classes with specific "
                             "constitutive models.");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  // The return stress increment classes are intended to be iterative materials, so must set compute
  // = false for all inheriting classes
  params.set<bool>("compute") = false;
  params.suppressParameter<bool>("compute");
  return params;
}

StressUpdateBase::StressUpdateBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_base_name + "elasticity_tensor")),
    _elastic_strain_old(getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "elastic_strain"))
{
}

void
StressUpdateBase::updateStress(RankTwoTensor & /*strain_increment*/,
                               RankTwoTensor & /*inelastic_strain_increment*/,
                               RankTwoTensor & /*stress_new*/)
{
}

void
StressUpdateBase::setQp(unsigned int qp)
{
  _qp = qp;
}

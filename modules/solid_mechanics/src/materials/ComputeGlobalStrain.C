//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGlobalStrain.h"
#include "GlobalStrainUserObjectInterface.h"

// MOOSE includes
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeGlobalStrain);

InputParameters
ComputeGlobalStrain::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material for storing the global strain values from the scalar variable");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addCoupledVar("scalar_global_strain", "Scalar variable for global strain");
  params.addCoupledVar("displacements", "The name of the displacement variables");
  params.addRequiredParam<UserObjectName>("global_strain_uo",
                                          "The name of the GlobalStrainUserObject");

  params.set<MooseEnum>("constant_on") = "SUBDOMAIN";
  return params;
}

ComputeGlobalStrain::ComputeGlobalStrain(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _scalar_global_strain(coupledScalarValue("scalar_global_strain")),
    _global_strain(declareProperty<RankTwoTensor>(_base_name + "global_strain")),
    _pst(getUserObject<GlobalStrainUserObjectInterface>("global_strain_uo")),
    _periodic_dir(_pst.getPeriodicDirections()),
    _dim(_mesh.dimension()),
    _ndisp(coupledComponents("displacements"))
{
}

void
ComputeGlobalStrain::initQpStatefulProperties()
{
  _global_strain[_qp].zero();
}

void
ComputeGlobalStrain::computeQpProperties()
{
  RankTwoTensor & strain = _global_strain[_qp];
  strain.fillFromScalarVariable(_scalar_global_strain);

  for (unsigned int dir = 0; dir < _dim; ++dir)
    if (!_periodic_dir(dir))
      for (unsigned int var = 0; var < _ndisp; ++var)
        strain(dir, var) = 0.0;
}

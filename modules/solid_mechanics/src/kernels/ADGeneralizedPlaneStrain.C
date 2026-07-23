//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ADGeneralizedPlaneStrain.h"

#include "Function.h"
#include "RankTwoTensor.h"

registerMooseObject("SolidMechanicsApp", ADGeneralizedPlaneStrain);

InputParameters
ADGeneralizedPlaneStrain::validParams()
{
  InputParameters params = ADKernelScalarBase::validParams();
  params.addClassDescription(
      "Assembles the generalized plane strain scalar equation using automatic differentiation.");
  params.renameCoupledVar("scalar_variable",
                          "scalar_out_of_plane_strain",
                          "Scalar variable for generalized plane strain");
  params.makeParamRequired<std::vector<VariableName>>("scalar_out_of_plane_strain");
  params.addParam<FunctionName>("out_of_plane_pressure_function",
                                "Function used to prescribe pressure (applied toward the body) in "
                                "the out-of-plane direction");
  params.addDeprecatedParam<FunctionName>(
      "out_of_plane_pressure",
      "Function used to prescribe pressure (applied toward the body) in the out-of-plane direction",
      "This has been replaced by 'out_of_plane_pressure_function'");
  params.addParam<MaterialPropertyName>("out_of_plane_pressure_material",
                                        "0",
                                        "Material used to prescribe pressure (applied toward the "
                                        "body) in the out-of-plane direction");
  MooseEnum out_of_plane_direction("x y z", "z");
  params.addParam<MooseEnum>(
      "out_of_plane_direction", out_of_plane_direction, "The direction of the out-of-plane strain");
  params.addDeprecatedParam<Real>(
      "factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)",
      "This has been replaced by 'pressure_factor'");
  params.addParam<Real>(
      "pressure_factor",
      "Scale factor applied to prescribed out-of-plane pressure (both material and function)");
  params.addParam<std::string>("base_name", "Material property base name");
  params.set<bool>("compute_field_residuals") = false;
  params.suppressParameter<bool>("compute_field_residuals");

  return params;
}

ADGeneralizedPlaneStrain::ADGeneralizedPlaneStrain(const InputParameters & parameters)
  : ADKernelScalarBase(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _stress(getADMaterialProperty<RankTwoTensor>(_base_name + "stress")),
    _out_of_plane_pressure_function(parameters.isParamSetByUser("out_of_plane_pressure_function")
                                        ? &getFunction("out_of_plane_pressure_function")
                                    : parameters.isParamSetByUser("out_of_plane_pressure")
                                        ? &getFunction("out_of_plane_pressure")
                                        : nullptr),
    _out_of_plane_pressure_material(getMaterialProperty<Real>("out_of_plane_pressure_material")),
    _pressure_factor(parameters.isParamSetByUser("pressure_factor")
                         ? getParam<Real>("pressure_factor")
                     : parameters.isParamSetByUser("factor") ? getParam<Real>("factor")
                                                             : 1.0),
    _out_of_plane_direction(getParam<MooseEnum>("out_of_plane_direction"))
{
  if (parameters.isParamSetByUser("out_of_plane_pressure_function") &&
      parameters.isParamSetByUser("out_of_plane_pressure"))
    paramError("out_of_plane_pressure_function",
               "Cannot specify both 'out_of_plane_pressure_function' and "
               "'out_of_plane_pressure'");
  if (parameters.isParamSetByUser("pressure_factor") && parameters.isParamSetByUser("factor"))
    paramError("pressure_factor", "Cannot specify both 'pressure_factor' and 'factor'");
}

void
ADGeneralizedPlaneStrain::initialSetup()
{
  if (getBlockCoordSystem() == Moose::COORD_RZ)
    _out_of_plane_direction = 1;
  else if (getBlockCoordSystem() != Moose::COORD_XYZ)
    paramError("out_of_plane_direction",
               "Generalized plane strain supports only Cartesian and axisymmetric coordinate "
               "systems");
}

ADReal
ADGeneralizedPlaneStrain::computeQpResidual()
{
  return 0;
}

ADReal
ADGeneralizedPlaneStrain::computeScalarQpResidual()
{
  const Real out_of_plane_pressure =
      ((_out_of_plane_pressure_function ? _out_of_plane_pressure_function->value(_t, _q_point[_qp])
                                        : 0.0) +
       _out_of_plane_pressure_material[_qp]) *
      _pressure_factor;

  return _stress[_qp](_out_of_plane_direction, _out_of_plane_direction) + out_of_plane_pressure;
}

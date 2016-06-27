/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputeStrainBase.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<ComputeStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredCoupledVar("displacements", "The displacements appropriate for the simulation geometry and coordinate system");
  params.addParam<std::string>("base_name", "Optional parameter that allows the user to define multiple mechanics material systems on the same block, i.e. for multiple phases");
  params.addPrivateParam<bool>("stateful_displacements", false);
  params.addParam<Real>("temperature_ref", 273, "Deprecated: Reference temperature for thermal expansion in K");
  params.addCoupledVar("temperature", 273, "Decprecated: Temperature in Kelvin");
  params.addParam<Real>("thermal_expansion_coeff", 0, "Deprecated: Thermal expansion coefficient in 1/K");
  params.addParam<bool>("volumetric_locking_correction", true, "Flag to correct volumetric locking");
  return params;
}

ComputeStrainBase::ComputeStrainBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _ndisp(coupledComponents("displacements")),
    _disp(3),
    _grad_disp(3),
    _grad_disp_old(3),
    _T(coupledValue("temperature")),
    _T0(getParam<Real>("temperature_ref")),
    _thermal_expansion_coeff(getParam<Real>("thermal_expansion_coeff")),
    _no_thermal_eigenstrains(false),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : "" ),
    _mechanical_strain(declareProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _total_strain(declareProperty<RankTwoTensor>(_base_name + "total_strain")),
    _stateful_displacements(getParam<bool>("stateful_displacements") && _fe_problem.isTransient()),
    _volumetric_locking_correction(getParam<bool>("volumetric_locking_correction"))
{
  // Checking for consistency between mesh size and length of the provided displacements vector
  if (_ndisp != _mesh.dimension())
    mooseError("The number of variables supplied in 'displacements' must match the mesh dimension.");

  if (parameters.isParamSetByUser("thermal_expansion_coeff"))
    _no_thermal_eigenstrains = true;

  if (_no_thermal_eigenstrains)
    mooseDeprecated("The calculation of the thermal strains has been moved to the material ComputeThermalExpansionEigenStrains");

  // fetch coupled variables and gradients (as stateful properties if necessary)
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp[i] = &coupledValue("displacements", i);
    _grad_disp[i] = &coupledGradient("displacements", i);

    if (_stateful_displacements)
      _grad_disp_old[i] = &coupledGradientOld("displacements" ,i);
    else
      _grad_disp_old[i] = &_grad_zero;
  }

  // set unused dimensions to zero
  for (unsigned i = _ndisp; i < 3; ++i)
  {
    _disp[i] = &_zero;
    _grad_disp[i] = &_grad_zero;
    _grad_disp_old[i] = &_grad_zero;
  }
}

void
ComputeStrainBase::initQpStatefulProperties()
{
  _mechanical_strain[_qp].zero();
  _total_strain[_qp].zero();
}

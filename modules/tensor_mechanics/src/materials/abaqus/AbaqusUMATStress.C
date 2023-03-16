//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUMATStress.h"
#include "StepUserObject.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "libmesh/int_range.h"
#include <string.h>
#include <algorithm>

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("TensorMechanicsApp", AbaqusUMATStress);

InputParameters
AbaqusUMATStress::validParams()
{
  InputParameters params = ComputeGeneralStressBase::validParams();
  params.addClassDescription("Coupling material to use Abaqus UMAT models in MOOSE");
  params.addRequiredParam<FileName>(
      "plugin", "The path to the compiled dynamic library for the plugin you want to use");
  params.addRequiredParam<bool>(
      "use_one_based_indexing",
      "Parameter to control whether indexing for element and integration points as presented to "
      "UMAT models is based on 1 (true) or 0 (false). This does not affect internal MOOSE "
      "numbering. The option to use 0-based numbering is deprecated and will be removed soon.");
  params.addRequiredParam<std::vector<Real>>(
      "constant_properties", "Constant mechanical and thermal material properties (PROPS)");
  params.addRequiredParam<unsigned int>("num_state_vars",
                                        "The number of state variables this UMAT is going to use");
  params.addCoupledVar("temperature", 0.0, "Coupled temperature");
  params.addCoupledVar("external_fields",
                       "The external fields that can be used in the UMAT subroutine");
  params.addParam<std::vector<MaterialPropertyName>>("external_properties", "");
  params.addParam<MooseEnum>("decomposition_method",
                             ComputeFiniteStrain::decompositionType(),
                             "Method to calculate the strain kinematics.");
  params.addParam<bool>(
      "use_displaced_mesh",
      false,
      "Whether or not this object should use the "
      "displaced mesh for computing displacements and quantities based on the deformed state.");
  params.addParam<UserObjectName>(
      "step_user_object", "The StepUserObject that provides times from simulation loading steps.");
  return params;
}

#ifndef METHOD
#error "The METHOD preprocessor symbol must be supplied by the build system."
#endif

AbaqusUMATStress::AbaqusUMATStress(const InputParameters & parameters)
  : ComputeGeneralStressBase(parameters),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _umat(_library.getFunction<umat_t>("umat_")),
    _aqNSTATV(getParam<unsigned int>("num_state_vars")),
    _aqSTATEV(_aqNSTATV),
    _aqPROPS(getParam<std::vector<Real>>("constant_properties")),
    _aqNPROPS(_aqPROPS.size()),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _strain_increment(getOptionalMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _Fbar(getOptionalMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _Fbar_old(getOptionalMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _state_var(declareProperty<std::vector<Real>>(_base_name + "state_var")),
    _state_var_old(getMaterialPropertyOld<std::vector<Real>>(_base_name + "state_var")),
    _elastic_strain_energy(declareProperty<Real>(_base_name + "elastic_strain_energy")),
    _plastic_dissipation(declareProperty<Real>(_base_name + "plastic_dissipation")),
    _creep_dissipation(declareProperty<Real>(_base_name + "creep_dissipation")),
    _material_timestep(declareProperty<Real>(_base_name + "material_timestep_limit")),
    _rotation_increment(
        getOptionalMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _temperature(coupledValue("temperature")),
    _temperature_old(coupledValueOld("temperature")),
    _external_fields(coupledValues("external_fields")),
    _external_fields_old(coupledValuesOld("external_fields")),
    _number_external_fields(_external_fields.size()),
    _external_property_names(getParam<std::vector<MaterialPropertyName>>("external_properties")),
    _number_external_properties(_external_property_names.size()),
    _external_properties(_number_external_properties),
    _external_properties_old(_number_external_properties),
    _use_one_based_indexing(getParam<bool>("use_one_based_indexing")),
    _decomposition_method(
        getParam<MooseEnum>("decomposition_method").getEnum<ComputeFiniteStrain::DecompMethod>())
{
  if (!_use_one_based_indexing)
    mooseDeprecated(
        "AbaqusUMATStress has transitioned to 1-based indexing in the element (NOEL) and "
        "integration point (NPT) numbers to ensure maximum compatibility with legacy UMAT files. "
        "Please ensure that any new UMAT plugins using these quantities are using the correct "
        "indexing. 0-based indexing will be deprecated soon.");

  // get material properties
  for (std::size_t i = 0; i < _number_external_properties; ++i)
  {
    _external_properties[i] = &getMaterialProperty<Real>(_external_property_names[i]);
    _external_properties_old[i] = &getMaterialPropertyOld<Real>(_external_property_names[i]);
  }

  // Read mesh dimension and size UMAT arrays (we always size for full 3D)
  _aqNTENS = 6; // Size of the stress or strain component array (NDI+NSHR)
  _aqNSHR = 3;  // Number of engineering shear stress components
  _aqNDI = 3;   // Number of direct stress components (always 3)

  _aqDDSDDT.resize(_aqNTENS);
  _aqDRPLDE.resize(_aqNTENS);
  _aqSTRAN.resize(_aqNTENS);
  _aqDFGRD0.resize(9);
  _aqDFGRD1.resize(9);
  _aqDROT.resize(9);
  _aqSTRESS.resize(_aqNTENS);
  _aqDDSDDE.resize(_aqNTENS * _aqNTENS);
  _aqDSTRAN.resize(_aqNTENS);
  _aqPREDEF.resize(_number_external_fields + _number_external_properties);
  _aqDPRED.resize(_number_external_fields + _number_external_properties);
}

void
AbaqusUMATStress::initialSetup()
{
  // The _Fbar, _Fbar_old, and _rotation_increment optional properties are only available when an
  // incremental strain formulation is used. If they are not avaliable we advide the user to
  // select an incremental formulation.
  if (!_strain_increment || !_Fbar || !_Fbar_old || !_rotation_increment)
    mooseError("AbaqusUMATStress '",
               name(),
               "': Incremental strain quantities are not available. You likely are using a total "
               "strain formulation. Specify `incremental = true` in the tensor mechanics action, "
               "or use ComputeIncrementalSmallStrain in your input file.");

  // Let's automatically detect uos and identify the one we are interested in.
  // If there is more than one, we assume something is off and error out.
  if (!isParamSetByUser("step_user_object"))
    getStepUserObject(_fe_problem, _step_user_object, name());
  else
    _step_user_object = &getUserObject<StepUserObject>("step_user_object");
}

void
AbaqusUMATStress::initQpStatefulProperties()
{
  ComputeGeneralStressBase::initQpStatefulProperties();

  // Initialize state variable vector
  _state_var[_qp].resize(_aqNSTATV);
  for (const auto i : make_range(_aqNSTATV))
    _state_var[_qp][i] = 0.0;
}

void
AbaqusUMATStress::computeProperties()
{
  // current element "number"
  _aqNOEL = _current_elem->id() + (_use_one_based_indexing ? 1 : 0);

  // characteristic element length
  _aqCELENT = std::pow(_current_elem->volume(), 1.0 / _current_elem->dim());

  if (!_step_user_object)
  {
    // Value of total time at the beginning of the current increment
    _aqTIME[0] = _t - _dt;
  }
  else
  {
    const unsigned int start_time_step = _step_user_object->getStep(_t - _dt);
    const Real step_time = _step_user_object->getStartTime(start_time_step);
    // Value of step time at the beginning of the current increment
    _aqTIME[0] = step_time;
  }
  // Value of total time at the beginning of the current increment
  _aqTIME[1] = _t - _dt;

  // Time increment
  _aqDTIME = _dt;

  // Fill unused characters with spaces (Fortran)
  std::fill(_aqCMNAME, _aqCMNAME + 80, ' ');
  std::memcpy(_aqCMNAME, name().c_str(), name().size());

  ComputeGeneralStressBase::computeProperties();
}

void
AbaqusUMATStress::computeQpStress()
{
  // C uses row-major, whereas Fortran uses column major
  // therefore, all unsymmetric matrices must be transposed before passing them to Fortran
  RankTwoTensor FBar_old_fortran = _Fbar_old[_qp].transpose();
  RankTwoTensor FBar_fortran = _Fbar[_qp].transpose();
  RankTwoTensor DROT_fortran = _rotation_increment[_qp].transpose();
  const Real * myDFGRD0 = &(FBar_old_fortran(0, 0));
  const Real * myDFGRD1 = &(FBar_fortran(0, 0));
  const Real * myDROT = &(DROT_fortran(0, 0));

  // copy because UMAT does not guarantee constness
  for (const auto i : make_range(9))
  {
    _aqDFGRD0[i] = myDFGRD0[i];
    _aqDFGRD1[i] = myDFGRD1[i];
    _aqDROT[i] = myDROT[i];
  }

  // Recover "old" state variables
  for (const auto i : make_range(_aqNSTATV))
    _aqSTATEV[i] = _state_var_old[_qp][i];

  // Pass through updated stress, total strain, and strain increment arrays
  static const std::array<Real, 6> strain_factor{{1, 1, 1, 2, 2, 2}};
  // Account for difference in vector order convention: yz, xz, xy (MOOSE)  vs xy, xz, yz
  // (commercial software)
  static const std::array<std::pair<unsigned int, unsigned int>, 6> component{
      {{0, 0}, {1, 1}, {2, 2}, {0, 1}, {0, 2}, {1, 2}}};

  // rotate old stress if HughesWinget
  RankTwoTensor stress_old = _stress_old[_qp];
  if (_decomposition_method == ComputeFiniteStrain::DecompMethod::HughesWinget)
    stress_old.rotate(_rotation_increment[_qp]);

  for (const auto i : make_range(_aqNTENS))
  {
    const auto a = component[i].first;
    const auto b = component[i].second;
    _aqSTRESS[i] = stress_old(a, b);
    _aqSTRAN[i] = _total_strain_old[_qp](a, b) * strain_factor[i];
    _aqDSTRAN[i] = _strain_increment[_qp](a, b) * strain_factor[i];
  }

  // current coordinates
  for (const auto i : make_range(Moose::dim))
    _aqCOORDS[i] = _q_point[_qp](i);

  // zero out Jacobian contribution
  for (const auto i : make_range(_aqNTENS * _aqNTENS))
    _aqDDSDDE[i] = 0.0;

  // Set PNEWDT initially to a large value
  _aqPNEWDT = std::numeric_limits<Real>::max();

  // Temperature
  _aqTEMP = _temperature_old[_qp];

  // Temperature increment
  _aqDTEMP = _temperature[_qp] - _temperature_old[_qp];

  for (const auto i : make_range(_number_external_fields))
  {
    // External field at this step
    _aqPREDEF[i] = (*_external_fields_old[i])[_qp];

    // External field increments
    _aqDPRED[i] = (*_external_fields[i])[_qp] - (*_external_fields_old[i])[_qp];
  }

  for (const auto i : make_range(_number_external_properties))
  {
    // External property at this step
    _aqPREDEF[i + _number_external_fields] = (*_external_properties_old[i])[_qp];

    // External property increments
    _aqDPRED[i + _number_external_fields] =
        (*_external_properties[i])[_qp] - (*_external_properties_old[i])[_qp];
  }

  // Layer number (not supported)
  _aqLAYER = -1;

  // Section point number within the layer (not supported)
  _aqKSPT = -1;

  // Increment number
  _aqKINC = _t_step;
  _aqKSTEP = 1;

  // integration point number
  _aqNPT = _qp + (_use_one_based_indexing ? 1 : 0);

  // Connection to extern statement
  _umat(_aqSTRESS.data(),
        _aqSTATEV.data(),
        _aqDDSDDE.data(),
        &_elastic_strain_energy[_qp],
        &_plastic_dissipation[_qp],
        &_creep_dissipation[_qp],
        &_aqRPL,
        _aqDDSDDT.data(),
        _aqDRPLDE.data(),
        &_aqDRPLDT,
        _aqSTRAN.data(),
        _aqDSTRAN.data(),
        _aqTIME.data(),
        &_aqDTIME,
        &_aqTEMP,
        &_aqDTEMP,
        _aqPREDEF.data(),
        _aqDPRED.data(),
        _aqCMNAME,
        &_aqNDI,
        &_aqNSHR,
        &_aqNTENS,
        &_aqNSTATV,
        _aqPROPS.data(),
        &_aqNPROPS,
        _aqCOORDS.data(),
        _aqDROT.data(),
        &_aqPNEWDT,
        &_aqCELENT,
        _aqDFGRD0.data(),
        _aqDFGRD1.data(),
        &_aqNOEL,
        &_aqNPT,
        &_aqLAYER,
        &_aqKSPT,
        &_aqKSTEP,
        &_aqKINC);

  // Update state variables
  for (int i = 0; i < _aqNSTATV; ++i)
    _state_var[_qp][i] = _aqSTATEV[i];

  // Here, we apply UMAT convention: Always multiply _dt by PNEWDT to determine the material time
  // step MOOSE time stepper will choose the most limiting of all material time step increments
  // provided
  _material_timestep[_qp] = _aqPNEWDT * _dt;

  // Get new stress tensor - UMAT should update stress
  // Account for difference in vector order convention: yz, xz, xy (MOOSE)  vs xy, xz, yz
  // (commercial software)
  _stress[_qp] = RankTwoTensor(
      _aqSTRESS[0], _aqSTRESS[1], _aqSTRESS[2], _aqSTRESS[5], _aqSTRESS[4], _aqSTRESS[3]);

  // Rotate the stress state to the current configuration, unless HughesWinget
  if (_decomposition_method != ComputeFiniteStrain::DecompMethod::HughesWinget)
    _stress[_qp].rotate(_rotation_increment[_qp]);

  // Build Jacobian matrix from UMAT's Voigt non-standard order to fourth order tensor.
  const unsigned int N = Moose::dim;
  const unsigned int ntens = N * (N + 1) / 2;
  const int nskip = N - 1;

  for (const auto i : make_range(N))
    for (const auto j : make_range(N))
      for (const auto k : make_range(N))
        for (const auto l : make_range(N))
        {
          if (i == j)
            _jacobian_mult[_qp](i, j, k, l) =
                k == l ? _aqDDSDDE[i * ntens + k] : _aqDDSDDE[i * ntens + k + nskip + l];
          else
            // i!=j
            _jacobian_mult[_qp](i, j, k, l) =
                k == l ? _aqDDSDDE[(nskip + i + j) * ntens + k]
                       : _aqDDSDDE[(nskip + i + j) * ntens + k + nskip + l];
        }
}

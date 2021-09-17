//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUMATStress.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include <string.h>
#include <algorithm>

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("TensorMechanicsApp", AbaqusUMATStress);

InputParameters
AbaqusUMATStress::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Coupling material to use Abaqus UMAT models in MOOSE");
  params.addRequiredParam<FileName>(
      "plugin", "The path to the compiled dynamic library for the plugin you want to use");
  params.addRequiredParam<std::vector<Real>>(
      "constant_properties", "Constant mechanical and thermal material properties (PROPS)");
  params.addRequiredParam<unsigned int>("num_state_vars",
                                        "The number of state variables this UMAT is going to use");
  params.addCoupledVar("temperature", 0.0, "Coupled temperature");

  params.addCoupledVar("external_fields",
                       "The external fields that can be used in the UMAT subroutine");

  return params;
}

#ifndef METHOD
#error "The METHOD preprocessor symbol must be supplied by the build system."
#endif

AbaqusUMATStress::AbaqusUMATStress(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _umat(_library.getFunction<umat_t>("umat_")),
    _aqNSTATV(getParam<unsigned int>("num_state_vars")),
    _aqSTATEV(_aqNSTATV),
    _aqPROPS(getParam<std::vector<Real>>("constant_properties")),
    _aqNPROPS(_aqPROPS.size()),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _total_strain_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "total_strain")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _Fbar(getMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _Fbar_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _state_var(declareProperty<std::vector<Real>>(_base_name + "state_var")),
    _state_var_old(getMaterialPropertyOld<std::vector<Real>>(_base_name + "state_var")),
    _elastic_strain_energy(declareProperty<Real>(_base_name + "elastic_strain_energy")),
    _plastic_dissipation(declareProperty<Real>(_base_name + "plastic_dissipation")),
    _creep_dissipation(declareProperty<Real>(_base_name + "creep_dissipation")),
    _material_timestep(declareProperty<Real>(_base_name + "material_timestep_limit")),
    _rotation_increment(getMaterialProperty<RankTwoTensor>(_base_name + "rotation_increment")),
    _temperature(coupledValue("temperature")),
    _temperature_old(coupledValueOld("temperature")),
    _external_fields(coupledValues("external_fields")),
    _external_fields_old(coupledValuesOld("external_fields")),
    _number_external_fields(_external_fields.size())
{
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
  _aqPREDEF.resize(_number_external_fields);
  _aqDPRED.resize(_number_external_fields);
}

void
AbaqusUMATStress::initQpStatefulProperties()
{
  ComputeStressBase::initQpStatefulProperties();

  // Initialize state variable vector
  _state_var[_qp].resize(_aqNSTATV);
  for (int i = 0; i < _aqNSTATV; ++i)
    _state_var[_qp][i] = 0.0;
}

void
AbaqusUMATStress::computeProperties()
{
  // current element "number"
  _aqNOEL = _current_elem->id();

  // characteristic element length
  _aqCELENT = std::pow(_current_elem->volume(), 1.0 / _current_elem->dim());

  // For now, total time and step time mean the exact same thing
  // Value of step time at the beginning of the current increment
  _aqTIME[0] = _t;
  // Value of total time at the beginning of the current increment
  _aqTIME[1] = _t;

  // Time increment
  _aqDTIME = _dt;

  // Fill unused characters with spaces (Fortran)
  std::fill(_aqCMNAME, _aqCMNAME + 80, ' ');
  std::memcpy(_aqCMNAME, name().c_str(), name().size());

  ComputeStressBase::computeProperties();
}

void
AbaqusUMATStress::computeQpStress()
{
  const Real * myDFGRD0 = &(_Fbar_old[_qp](0, 0));
  const Real * myDFGRD1 = &(_Fbar[_qp](0, 0));
  const Real * myDROT = &(_rotation_increment[_qp](0, 0));

  // copy because UMAT does not guarantee constness
  for (unsigned int i = 0; i < 9; ++i)
  {
    _aqDFGRD0[i] = myDFGRD0[i];
    _aqDFGRD1[i] = myDFGRD1[i];
    _aqDROT[i] = myDROT[i];
  }

  // Recover "old" state variables
  for (int i = 0; i < _aqNSTATV; ++i)
    _aqSTATEV[i] = _state_var_old[_qp][i];

  // Pass through updated stress, total strain, and strain increment arrays
  static const std::array<Real, 6> strain_factor{{1, 1, 1, 2, 2, 2}};
  static const std::array<std::pair<unsigned int, unsigned int>, 6> component{
      {{0, 0}, {1, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}}};

  for (int i = 0; i < _aqNTENS; ++i)
  {
    _aqSTRESS[i] = _stress_old[_qp](component[i].first, component[i].second);
    _aqSTRAN[i] =
        _total_strain_old[_qp](component[i].first, component[i].second) * strain_factor[i];
    _aqDSTRAN[i] =
        _strain_increment[_qp](component[i].first, component[i].second) * strain_factor[i];
  }

  // current coordinates
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    _aqCOORDS[i] = _q_point[_qp](i);

  // zero out Jacobian contribution
  for (int i = 0; i < _aqNTENS * _aqNTENS; ++i)
    _aqDDSDDE[i] = 0.0;

  // Set PNEWDT initially to a large value
  _aqPNEWDT = std::numeric_limits<Real>::max();

  // Temperature
  _aqTEMP = _temperature[_qp];

  // Temperature increment
  _aqDTEMP = _temperature[_qp] - _temperature_old[_qp];

  for (unsigned int i = 0; i < _number_external_fields; i++)
  {
    // External field at this step
    _aqPREDEF[i] = (*_external_fields[i])[_qp];

    // External field increments
    _aqDPRED[i] = (*_external_fields[i])[_qp] - (*_external_fields_old[i])[_qp];
  }

  // Layer number (not supported)
  _aqLAYER = -1;

  // Section point number within the layer (not supported)
  _aqKSPT = -1;

  // Increment number
  _aqKINC = _t_step;
  _aqKSTEP = 1;

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
        &_qp,
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
  _stress[_qp] = RankTwoTensor(
      _aqSTRESS[0], _aqSTRESS[1], _aqSTRESS[2], _aqSTRESS[3], _aqSTRESS[4], _aqSTRESS[5]);

  // use DDSDDE as Jacobian mult
  _jacobian_mult[_qp].fillSymmetric21FromInputVector(std::array<Real, 21>{{
      _aqDDSDDE[0],  // C1111
      _aqDDSDDE[1],  // C1122
      _aqDDSDDE[2],  // C1133
      _aqDDSDDE[3],  // C1123
      _aqDDSDDE[4],  // C1113
      _aqDDSDDE[5],  // C1112
      _aqDDSDDE[7],  // C2222
      _aqDDSDDE[8],  // C2233
      _aqDDSDDE[9],  // C2223
      _aqDDSDDE[10], // C2213
      _aqDDSDDE[11], // C2212
      _aqDDSDDE[14], // C3333
      _aqDDSDDE[15], // C3323
      _aqDDSDDE[16], // C3313
      _aqDDSDDE[17], // C3312
      _aqDDSDDE[21], // C2323
      _aqDDSDDE[22], // C2313
      _aqDDSDDE[23], // C2312
      _aqDDSDDE[28], // C1313
      _aqDDSDDE[29], // C1312
      _aqDDSDDE[35]  // C1212
  }});
}

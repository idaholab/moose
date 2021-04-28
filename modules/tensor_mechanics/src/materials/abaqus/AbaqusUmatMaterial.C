//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUmatMaterial.h"
#include "Factory.h"
#include "MooseMesh.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

#ifdef LIBMESH_HAVE_DLOPEN
#include <dlfcn.h>
#endif

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("TensorMechanicsApp", AbaqusUmatMaterial);

InputParameters
AbaqusUmatMaterial::validParams()
{
  InputParameters params = ComputeStressBase::validParams();
  params.addClassDescription("Coupling material to use Abaqus UMAT models in MOOSE");
  params.addRequiredParam<FileName>(
      "plugin", "The path to the compiled dynamic library for the plugin you want to use");
  params.addRequiredParam<std::vector<Real>>("mechanical_constants",
                                             "Mechanical Material Properties");
  params.addParam<std::vector<Real>>("thermal_constants", "Thermal Material Properties");
  params.addRequiredParam<unsigned int>("num_state_vars",
                                        "The number of state variables this UMAT is going to use");
  return params;
}

AbaqusUmatMaterial::AbaqusUmatMaterial(const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _plugin(getParam<FileName>("plugin")),
    _mechanical_constants(getParam<std::vector<Real>>("mechanical_constants")),
    _thermal_constants(getParam<std::vector<Real>>("thermal_constants")),
    _num_state_vars(getParam<unsigned int>("num_state_vars")),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _total_strain(getMaterialProperty<RankTwoTensor>(_base_name + "total_strain")),
    _strain_increment(getMaterialProperty<RankTwoTensor>(_base_name + "strain_increment")),
    _jacobian_mult(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult")),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _Fbar(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _Fbar_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "deformation_gradient")),
    _state_var(declareProperty<std::vector<Real>>(_base_name + "state_var")),
    _state_var_old(getMaterialPropertyOld<std::vector<Real>>(_base_name + "state_var")),
    _elastic_strain_energy(declareProperty<Real>(_base_name + "elastic_strain_energy")),
    _plastic_dissipation(declareProperty<Real>(_base_name + "plastic_dissipation")),
    _creep_dissipation(declareProperty<Real>(_base_name + "creep_dissipation"))
{
#if defined(METHOD)
  _plugin += std::string("-") + QUOTE(METHOD) + ".plugin";
#endif

  // Size and create full (mechanical+thermal) material property array
  _num_props = _mechanical_constants.size() + _thermal_constants.size();
  std::vector<Real> props_array(_num_props);
  for (unsigned int i = 0; i < _mechanical_constants.size(); ++i)
    props_array[i] = _mechanical_constants[i];
  for (unsigned int i = _mechanical_constants.size(); i < _num_props; ++i)
    props_array[i] = _thermal_constants[i];

  // Read mesh dimension and size UMAT arrays (we always size for full 3D)
  _aqNTENS = 6; // Size of the stress or strain component array (NDI+NSHR)
  _aqNSHR = 3;  // Number of engineering shear stress components
  _aqNDI = 3;   // Number of direct stress components (always 3)

  _aqSTATEV.resize(_num_state_vars);
  _aqDDSDDT.resize(_aqNTENS);
  _aqDRPLDE.resize(_aqNTENS);
  _aqSTRAN.resize(_aqNTENS);
  _aqDFGRD0.resize(9);
  _aqDFGRD1.resize(9);
  _aqSTRESS.resize(_aqNTENS);
  _aqDDSDDE.resize(_aqNTENS * _aqNTENS);
  _aqDSTRAN.resize(_aqNTENS);
  _aqPROPS.resize(_num_props);

  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _aqSTATEV[i] = 0.0;

  for (int i = 0; i < _aqNTENS; ++i)
  {
    _aqDDSDDT[i] = 0.0;
    _aqDRPLDE[i] = 0.0;
    _aqSTRAN[i] = 0.0;
    _aqSTRESS[i] = 0.0;
    _aqDSTRAN[i] = 0.0;
  }

  for (unsigned int i = 0; i < 9; ++i)
  {
    _aqDFGRD0[i] = 0.0;
    _aqDFGRD1[i] = 0.0;
  }

  for (int i = 0; i < _aqNTENS * _aqNTENS; ++i)
    _aqDDSDDE[i] = 0.0;

  // Assign materials properties from vector form into an array
  for (unsigned int i = 0; i < _num_props; ++i)
    _aqPROPS[i] = props_array[i];

  // Size UMAT state variable (NSTATV) and material constant (NPROPS) arrays
  _aqNSTATV = _num_state_vars;
  _aqNPROPS = _num_props;

  // Open the library
#ifdef LIBMESH_HAVE_DLOPEN
  _handle = dlopen(_plugin.c_str(), RTLD_LAZY);

  if (!_handle)
    mooseError("Cannot open library: ", dlerror());

  // Reset errors
  dlerror();

  // Snag the function pointer from the library
  {
    void * pointer = dlsym(_handle, "umat_");
    _umat = *reinterpret_cast<umat_t *>(&pointer);
  }

  // Catch errors
  const char * dlsym_error = dlerror();
  if (dlsym_error)
  {
    dlclose(_handle);
    std::ostringstream error;
    error << "Cannot load symbol 'umat_': " << dlsym_error << '\n';
    mooseError(error.str());
  }
#else
  mooseError("AbaqusUmatMaterial is not supported on Windows.");
#endif
}

AbaqusUmatMaterial::~AbaqusUmatMaterial()
{
#ifdef LIBMESH_HAVE_DLOPEN
  dlclose(_handle);
#endif
}

void
AbaqusUmatMaterial::initQpStatefulProperties()
{
  // Initialize state variable vector
  _state_var[_qp].resize(_num_state_vars);
  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _state_var[_qp][i] = 0.0;
}

void
AbaqusUmatMaterial::computeQpStress()
{
  Real myDFGRD0[9] = {_Fbar_old[_qp](0, 0),
                      _Fbar_old[_qp](1, 0),
                      _Fbar_old[_qp](2, 0),
                      _Fbar_old[_qp](0, 1),
                      _Fbar_old[_qp](1, 1),
                      _Fbar_old[_qp](2, 1),
                      _Fbar_old[_qp](0, 2),
                      _Fbar_old[_qp](1, 2),
                      _Fbar_old[_qp](2, 2)};
  Real myDFGRD1[9] = {_Fbar[_qp](0, 0),
                      _Fbar[_qp](1, 0),
                      _Fbar[_qp](2, 0),
                      _Fbar[_qp](0, 1),
                      _Fbar[_qp](1, 1),
                      _Fbar[_qp](2, 1),
                      _Fbar[_qp](0, 2),
                      _Fbar[_qp](1, 2),
                      _Fbar[_qp](2, 2)};

  for (unsigned int i = 0; i < 9; ++i)
  {
    _aqDFGRD0[i] = myDFGRD0[i];
    _aqDFGRD1[i] = myDFGRD1[i];
  }

  // Recover "old" state variables
  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _aqSTATEV[i] = _state_var_old[_qp][i];

  // Pass through updated stress, total strain, and strain increment arrays
  static const std::vector<std::pair<unsigned int, unsigned int>> component{
      {0, 0}, {1, 1}, {2, 2}, {1, 2}, {0, 2}, {0, 1}};
  for (int i = 0; i < _aqNTENS; ++i) // this works only for 3D
  {
    _aqSTRESS[i] = _stress_old[_qp](component[i].first, component[i].second);
    _aqSTRAN[i] = _total_strain[_qp](component[i].first, component[i].second);
    _aqDSTRAN[i] = _strain_increment[_qp](component[i].first, component[i].second);
  }

  // Pass through step , time, and coordinate system information
  _aqKSTEP = _t_step;    // Step number
  _aqTIME[0] = _t;       // Value of step time at the beginning of the current increment - Check
  _aqTIME[1] = _t - _dt; // Value of total time at the beginning of the current increment - Check
  _aqDTIME = _dt;        // Time increment
  for (unsigned int i = 0; i < 3; ++i) // Loop current coordinates in UMAT COORDS
    _aqCOORDS[i] = _q_point[_qp](i);

  // Connection to extern statement
  _umat(_aqSTRESS.data(),
        _aqSTATEV.data(),
        _aqDDSDDE.data(),
        &_aqSSE,
        &_aqSPD,
        &_aqSCD,
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
        &_aqPREDEF,
        &_aqDPRED,
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

  // Energy outputs
  _elastic_strain_energy[_qp] = _aqSSE;
  _plastic_dissipation[_qp] = _aqSPD;
  _creep_dissipation[_qp] = _aqSCD;

  // Update state variables
  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _state_var[_qp][i] = _aqSTATEV[i];

  // Get new stress tensor - UMAT should update stress
  _stress[_qp] = RankTwoTensor(
      _aqSTRESS[0], _aqSTRESS[1], _aqSTRESS[2], _aqSTRESS[3], _aqSTRESS[4], _aqSTRESS[5]);

  // update Jacobian multiplier
  if (_fe_problem.currentlyComputingJacobian())
    _jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

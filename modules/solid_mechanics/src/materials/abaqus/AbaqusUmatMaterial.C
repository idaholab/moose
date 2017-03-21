/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "AbaqusUmatMaterial.h"
#include "Factory.h"
#include "MooseMesh.h"

#include <dlfcn.h>
#define QUOTE(macro) stringifyName(macro)

template <>
InputParameters
validParams<AbaqusUmatMaterial>()
{
  InputParameters params = validParams<SolidModel>();
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
  : SolidModel(parameters),
    _plugin(getParam<FileName>("plugin")),
    _mechanical_constants(getParam<std::vector<Real>>("mechanical_constants")),
    _thermal_constants(getParam<std::vector<Real>>("thermal_constants")),
    _num_state_vars(getParam<unsigned int>("num_state_vars")),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(coupledGradient("disp_z")),
    _grad_disp_x_old(coupledGradientOld("disp_x")),
    _grad_disp_y_old(coupledGradientOld("disp_y")),
    _grad_disp_z_old(coupledGradientOld("disp_z")),
    _state_var(declareProperty<std::vector<Real>>("state_var")),
    _state_var_old(declarePropertyOld<std::vector<Real>>("state_var")),
    _Fbar(declareProperty<ColumnMajorMatrix>("Fbar")),
    _Fbar_old(declarePropertyOld<ColumnMajorMatrix>("Fbar")),
    _elastic_strain_energy(declareProperty<Real>("elastic_strain_energy")),
    _plastic_dissipation(declareProperty<Real>("plastic_dissipation")),
    _creep_dissipation(declareProperty<Real>("creep_dissipation"))
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

  // Read mesh dimension and size UMAT arrays
  if (_mesh.dimension() == 3) // 3D case
  {
    _NTENS = 6; // Size of the stress or strain component array (NDI+NSHR)
    _NSHR = 3;  // Number of engineering shear stress components
    _NDI = 3;   // Number of direct stress components (always 3)
  }
  else if (_mesh.dimension() == 2) // 2D case
  {
    _NTENS = 4;
    _NSHR = 1;
    _NDI = 3;
  }

  _STATEV = new Real[_num_state_vars];
  _DDSDDT = new Real[_NTENS];
  _DRPLDE = new Real[_NTENS];
  _STRAN = new Real[_NTENS];
  _DFGRD0 = new Real[9];
  _DFGRD1 = new Real[9];
  _STRESS = new Real[_NTENS];
  _DDSDDE = new Real[_NTENS * _NTENS];
  _DSTRAN = new Real[_NTENS];
  _PROPS = new Real[_num_props];

  for (unsigned int i = 0; i < _num_state_vars; ++i)
  {
    _STATEV[i] = 0.0;
  }

  for (int i = 0; i < _NTENS; ++i)
  {
    _DDSDDT[i] = 0.0;
    _DRPLDE[i] = 0.0;
    _STRAN[i] = 0.0;
    _STRESS[i] = 0.0;
    _DSTRAN[i] = 0.0;
  }

  for (unsigned int i = 0; i < 9; ++i)
  {
    _DFGRD0[i] = 0.0;
    _DFGRD1[i] = 0.0;
  }

  for (int i = 0; i < _NTENS * _NTENS; ++i)
  {
    _DDSDDE[i] = 0.0;
  }

  // Assign materials properties from vector form into an array
  for (unsigned int i = 0; i < _num_props; ++i)
  {
    _PROPS[i] = props_array[i];
  }

  // Size UMAT state variable (NSTATV) and material constant (NPROPS) arrays
  _NSTATV = _num_state_vars;
  _NPROPS = _num_props;

  // Open the library
  _handle = dlopen(_plugin.c_str(), RTLD_LAZY);

  if (!_handle)
  {
    std::ostringstream error;
    error << "Cannot open library: " << dlerror() << '\n';
    mooseError(error.str());
  }

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
}

AbaqusUmatMaterial::~AbaqusUmatMaterial()
{
  delete _STATEV;
  delete _DDSDDT;
  delete _DRPLDE;
  delete _STRAN;
  delete _DFGRD0;
  delete _DFGRD1;
  delete _STRESS;
  delete _DDSDDE;
  delete _DSTRAN;
  delete _PROPS;

  dlclose(_handle);
}

void
AbaqusUmatMaterial::initQpStatefulProperties()
{
  // Initialize state variable vector
  _state_var[_qp].resize(_num_state_vars);
  _state_var_old[_qp].resize(_num_state_vars);
  for (unsigned int i = 0; i < _num_state_vars; ++i)
  {
    _state_var[_qp][i] = 0.0;
    _state_var_old[_qp][i] = 0.0;
  }
}

void
AbaqusUmatMaterial::computeStress()
{
  // Calculate deformation gradient - modeled from "solid_mechanics/src/materials/Nonlinear3D.C"
  // Fbar = 1 + grad(u(k))
  ColumnMajorMatrix Fbar;

  Fbar(0, 0) = _grad_disp_x[_qp](0);
  Fbar(0, 1) = _grad_disp_x[_qp](1);
  Fbar(0, 2) = _grad_disp_x[_qp](2);
  Fbar(1, 0) = _grad_disp_y[_qp](0);
  Fbar(1, 1) = _grad_disp_y[_qp](1);
  Fbar(1, 2) = _grad_disp_y[_qp](2);
  Fbar(2, 0) = _grad_disp_z[_qp](0);
  Fbar(2, 1) = _grad_disp_z[_qp](1);
  Fbar(2, 2) = _grad_disp_z[_qp](2);

  Fbar.addDiag(1);
  _Fbar[_qp] = Fbar;

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
    _DFGRD0[i] = myDFGRD0[i];
    _DFGRD1[i] = myDFGRD1[i];
  }

  // Recover "old" state variables
  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _STATEV[i] = _state_var_old[_qp][i];

  // Pass through updated stress, total strain, and strain increment arrays
  for (int i = 0; i < _NTENS; ++i)
  {
    _STRESS[i] = _stress_old.component(i);
    _STRAN[i] = _total_strain[_qp].component(i);
    _DSTRAN[i] = _strain_increment.component(i);
  }

  // Pass through step , time, and coordinate system information
  _KSTEP = _t_step;    // Step number
  _TIME[0] = _t;       // Value of step time at the beginning of the current increment - Check
  _TIME[1] = _t - _dt; // Value of total time at the beginning of the current increment - Check
  _DTIME = _dt;        // Time increment
  for (unsigned int i = 0; i < 3; ++i) // Loop current coordinates in UMAT COORDS
    _COORDS[i] = _coord[i];

  // Connection to extern statement
  _umat(_STRESS,
        _STATEV,
        _DDSDDE,
        &_SSE,
        &_SPD,
        &_SCD,
        &_RPL,
        _DDSDDT,
        _DRPLDE,
        &_DRPLDT,
        _STRAN,
        _DSTRAN,
        _TIME,
        &_DTIME,
        &_TEMP,
        &_DTEMP,
        _PREDEF,
        _DPRED,
        &_CMNAME,
        &_NDI,
        &_NSHR,
        &_NTENS,
        &_NSTATV,
        _PROPS,
        &_NPROPS,
        _COORDS,
        _DROT,
        &_PNEWDT,
        &_CELENT,
        _DFGRD0,
        _DFGRD1,
        &_NOEL,
        &_NPT,
        &_LAYER,
        &_KSPT,
        &_KSTEP,
        &_KINC);

  // Energy outputs
  _elastic_strain_energy[_qp] = _SSE;
  _plastic_dissipation[_qp] = _SPD;
  _creep_dissipation[_qp] = _SCD;

  // Update state variables
  for (unsigned int i = 0; i < _num_state_vars; ++i)
    _state_var[_qp][i] = _STATEV[i];

  // Get new stress tensor - UMAT should update stress
  SymmTensor stressnew(_STRESS[0], _STRESS[1], _STRESS[2], _STRESS[3], _STRESS[4], _STRESS[5]);
  _stress[_qp] = stressnew;
}

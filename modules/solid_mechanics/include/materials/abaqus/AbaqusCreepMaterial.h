/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidModel.h"

#ifndef ABAQUSCREEPMATERIAL_H
#define ABAQUSCREEPMATERIAL_H

typedef void (*creep_t)(Real DECRA[],
                        Real DESWA[],
                        Real STATEV[],
                        Real * SERD,
                        Real EC[],
                        Real ESW[],
                        Real * P,
                        Real * QTILD,
                        Real * TEMP,
                        Real * DTEMP,
                        Real PREDEF[],
                        Real DPRED[],
                        Real TIME[],
                        Real * DTIME,
                        Real * CMNAME,
                        Real * LEXIMP,
                        Real * LEND,
                        Real COORDS[],
                        Real * NSTATV,
                        int * NOEL,
                        int * NPT,
                        int * LAYER,
                        int * KSPT,
                        int * KSTEP,
                        int * KINC);

// Forward Declaration
class AbaqusCreepMaterial;

template <>
InputParameters validParams<AbaqusCreepMaterial>();

// class define a property
// class AbaqusCreepMaterial : public VolumetricModel
class AbaqusCreepMaterial : public SolidModel
{
public:
  AbaqusCreepMaterial(const InputParameters & parameters);

  virtual ~AbaqusCreepMaterial();

protected:
  FileName _plugin;
  Real _youngs_modulus, _poissons_ratio;
  unsigned int _num_state_vars, _integration_flag, _solve_definition, _routine_flag;

  // The plugin library handle
  void * _handle;

  // Function pointer to the dynamically loaded function
  creep_t _creep;

  // CREEP subroutine real scalar values
  Real _SERD, _P, _QTILD, _TEMP, _DTEMP, _DTIME, _CMNAME, _LEXIMP, _LEND, _NSTATV;

  // CREEP subroutine arrays
  Real _DECRA[5], _DESWA[5], *_STATEV, _PREDEF[1], _DPRED[1], _TIME[2], _EC[2], _ESW[2], _COORDS[3];

  // CREEP subroutine Integer values
  int _NOEL, _NPT, _LAYER, _KSPT, _KSTEP, _KINC;

  // Elasticity reference
  Real _ebulk3, _eg2, _eg, _eg3, _elam, _elasticity_tensor[3], _stress_component[6];

  virtual void initStatefulProperties(unsigned n_points);
  // virtual void modifyStrain(const unsigned int qp,
  //                           const Real /*scale_factor*/,
  //                           SymmTensor & strain_increment,
  //                           SymmTensor & dstrain_increment_dT);
  void computeStress();

  MaterialProperty<std::vector<Real>> & _state_var;
  MaterialProperty<std::vector<Real>> & _state_var_old;
  MaterialProperty<SymmTensor> & _trial_stress;
  MaterialProperty<SymmTensor> & _trial_stress_old;
  MaterialProperty<SymmTensor> & _dev_trial_stress;
  MaterialProperty<SymmTensor> & _dev_trial_stress_old;
  MaterialProperty<Real> & _ets;
  MaterialProperty<Real> & _ets_old;
  MaterialProperty<SymmTensor> & _stress;
  MaterialProperty<SymmTensor> & _stress_old;
  MaterialProperty<Real> & _creep_inc;
  MaterialProperty<Real> & _creep_inc_old;
  MaterialProperty<Real> & _total_creep;
  MaterialProperty<Real> & _total_creep_old;
  MaterialProperty<Real> & _swell_inc;
  MaterialProperty<Real> & _swell_inc_old;
  MaterialProperty<Real> & _total_swell;
  MaterialProperty<Real> & _total_swell_old;
};

#endif // ABAQUSCREEPMATERIAL_H

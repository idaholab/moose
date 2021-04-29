//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeStressBase.h"

typedef void (*umat_t)(Real STRESS[],
                       Real STATEV[],
                       Real DDSDDE[],
                       Real * SSE,
                       Real * SPD,
                       Real * SCD,
                       Real * RPL,
                       Real DDSDDT[],
                       Real DRPLDE[],
                       Real * DRPLDT,
                       Real STRAN[],
                       Real DSTRAN[],
                       Real TIME[],
                       Real * DTIME,
                       Real * TEMP,
                       Real * DTEMP,
                       Real PREDEF[],
                       Real DPRED[],
                       char * CMNAME,
                       int * NDI,
                       int * NSHR,
                       int * NTENS,
                       int * NSTATV,
                       Real PROPS[],
                       int * NPROPS,
                       Real COORDS[],
                       Real DROT[],
                       Real * PNEWDT,
                       Real * CELENT,
                       Real DFGRD0[],
                       Real DFGRD1[],
                       int * NOEL,
                       int * NPT,
                       int * LAYER,
                       int * KSPT,
                       int * KSTEP,
                       int * KINC);

/**
 * Coupling material to use Abaqus UMAT models in MOOSE
 */
class AbaqusUmatMaterial : public ComputeStressBase
{
public:
  static InputParameters validParams();

  AbaqusUmatMaterial(const InputParameters & parameters);
  virtual ~AbaqusUmatMaterial();

protected:
  FileName _plugin;
  std::vector<Real> _mechanical_constants;
  std::vector<Real> _thermal_constants;
  unsigned int _num_state_vars;
  unsigned int _num_props;

  // The plugin library handle
  void * _handle;

  // Function pointer to the dynamically loaded function
  umat_t _umat;

  // UMAT real scalar values
  Real _aqSSE, _aqSPD, _aqSCD, _aqDRPLDT, _aqRPL, _aqPNEWDT, _aqDTIME, _aqTEMP, _aqDTEMP, _aqCELENT;

  // model name buffer
  char _aqCMNAME[80];

  // UMAT integer values
  int _aqNDI, _aqNSHR, _aqNTENS, _aqNSTATV, _aqNPROPS, _aqNOEL, _aqNPT, _aqLAYER, _aqKSPT, _aqKSTEP,
      _aqKINC;

  // UMAT arrays
  std::vector<Real> _aqSTATEV, _aqDDSDDT, _aqDRPLDE, _aqSTRAN, _aqDFGRD0, _aqDFGRD1, _aqSTRESS,
      _aqDDSDDE, _aqDSTRAN, _aqPROPS;
  std::array<Real, 2> _aqTIME;
  std::array<Real, 3> _aqCOORDS;
  std::array<Real, 3 * 3> _aqDROT;

  // single element "arrays"
  Real _aqPREDEF, _aqDPRED;

  virtual void initQpStatefulProperties();
  virtual void computeQpStress();

  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Jacobian multiplier (we approximate this using the elasticity tensor)
  MaterialProperty<RankFourTensor> & _jacobian_mult;

  const MaterialProperty<RankTwoTensor> & _Fbar;
  const MaterialProperty<RankTwoTensor> & _Fbar_old;

  MaterialProperty<std::vector<Real>> & _state_var;
  const MaterialProperty<std::vector<Real>> & _state_var_old;

  MaterialProperty<Real> & _elastic_strain_energy;
  MaterialProperty<Real> & _plastic_dissipation;
  MaterialProperty<Real> & _creep_dissipation;
};

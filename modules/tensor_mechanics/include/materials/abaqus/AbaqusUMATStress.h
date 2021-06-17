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
                       unsigned int * NPT,
                       int * LAYER,
                       int * KSPT,
                       int * KSTEP,
                       int * KINC);

/**
 * Coupling material to use Abaqus UMAT models in MOOSE
 */
class AbaqusUMATStress : public ComputeStressBase
{
public:
  static InputParameters validParams();

  AbaqusUMATStress(const InputParameters & parameters);
  virtual ~AbaqusUMATStress();

  /// perform per-element computation/initialization
  void computeProperties() override;

protected:
  // The plugin file name
  FileName _plugin;

  // The plugin library handle
  void * _handle;

  // Function pointer to the dynamically loaded function
  umat_t _umat;

  /// specific elastic strain energy
  Real _aqSSE;
  /// plastic dissipation
  Real _aqSPD;
  /// creep dissipation
  Real _aqSCD;

  /**
   * Volumetric heat generation per unit time at the end of the increment caused by mechanical
   * working of the material.
   */
  Real _aqRPL;

  ///  Variation of the volumetric heat generation (RPL) with respect to the temperature.
  Real _aqDRPLDT;

  /// Ratio of suggested new time increment to the time increment being used (out)
  Real _aqPNEWDT;

  /// Time increment
  Real _aqDTIME;

  /// Temperature at the start of the increment.
  Real _aqTEMP;

  /// Increment of temperature
  Real _aqDTEMP;

  /// Characteristic element length, which is a typical length of a line across an element for a first-order element (unused, set to 1)
  Real _aqCELENT;

  /// model name buffer
  char _aqCMNAME[80];

  /// Number of direct stress components at this point
  int _aqNDI;

  /// Number of engineering shear stress components at this point
  int _aqNSHR;

  /// Size of the stress or strain component array (NDI + NSHR).
  int _aqNTENS;

  /// Number of solution-dependent state variables that are associated with this material type
  int _aqNSTATV;

  /// Element number
  int _aqNOEL;

  /// Layer number (for composite shells and layered solids). (unused)
  int _aqLAYER;

  /// Section point number within the current layer. (unused)
  int _aqKSPT;

  /// Increment number (unused)
  int _aqKINC;

  // An array containing the solution-dependent state variables
  std::vector<Real> _aqSTATEV;

  /// Variation of the stress increments with respect to the temperature
  std::vector<Real> _aqDDSDDT;

  /// Variation of RPL with respect to the strain increments
  std::vector<Real> _aqDRPLDE;

  /// An array containing the total strains at the beginning of the increment
  std::vector<Real> _aqSTRAN;

  /// Array containing the deformation gradient at the beginning of the increment
  std::vector<Real> _aqDFGRD0;

  /// Array containing the deformation gradient at the end of the increment
  std::vector<Real> _aqDFGRD1;

  /// Stress tensor (in: old stress, out: updated stress)
  std::vector<Real> _aqSTRESS;

  /// Jacobian matrix of the model (out)
  std::vector<Real> _aqDDSDDE;

  /**
   * Array of strain increments. If thermal expansion is included in the same material definition,
   * these are the mechanical strain increments (the total strain increments minus the thermal
   * strain increments).
   */
  std::vector<Real> _aqDSTRAN;

  /// User-specified array of material constants associated with this user material
  std::vector<Real> _aqPROPS;

  /// Value of step time at the beginning of the current increment, total time at the beginning of the current increment
  std::array<Real, 2> _aqTIME;

  /// An array containing the coordinates of this point
  std::array<Real, 3> _aqCOORDS;

  /// Rotation increment matrix
  std::array<Real, 3 * 3> _aqDROT;

  /// User-defined number of material constants associated with this user material.
  int _aqNPROPS;

  /// Array of interpolated values of predefined field variables at this point at the start of the increment, based on the values read in at the nodes.
  Real _aqPREDEF;

  /// Array of increments of predefined field variables
  Real _aqDPRED;

  void initQpStatefulProperties() override;
  void computeQpStress() override;

  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _total_strain;
  const MaterialProperty<RankTwoTensor> & _strain_increment;

  /// Jacobian multiplier
  MaterialProperty<RankFourTensor> & _jacobian_mult;

  const MaterialProperty<RankTwoTensor> & _Fbar;
  const MaterialProperty<RankTwoTensor> & _Fbar_old;

  MaterialProperty<std::vector<Real>> & _state_var;
  const MaterialProperty<std::vector<Real>> & _state_var_old;

  MaterialProperty<Real> & _elastic_strain_energy;
  MaterialProperty<Real> & _plastic_dissipation;
  MaterialProperty<Real> & _creep_dissipation;

  /// recommended maximum timestep for this model under the current conditions
  MaterialProperty<Real> & _material_timestep;
};

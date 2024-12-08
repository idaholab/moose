//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeGeneralStressBase.h"
#include "DynamicLibraryLoader.h"
#include "ComputeFiniteStrain.h"
#include "RotationTensor.h"
#include "StepUOInterface.h"

class StepUserObject;

/**
 * Coupling material to use Abaqus UMAT models in MOOSE
 */
class AbaqusUMATStress : public ComputeGeneralStressBase, public StepUOInterface
{
public:
  static InputParameters validParams();

  AbaqusUMATStress(const InputParameters & parameters);

  /// check optional material properties for consistency
  void initialSetup() override;

  /// perform per-element computation/initialization
  void computeProperties() override;

protected:
  /// function type for the external UMAT function
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

  // The plugin file name
  FileName _plugin;

  // The plugin library wrapper
  DynamicLibraryLoader _library;

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

  /// Model name buffer
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

  /// Integration point number
  unsigned int _aqNPT;

  /// Layer number (for composite shells and layered solids). (not supported)
  int _aqLAYER;

  /// Section point number within the current layer. (not supported)
  int _aqKSPT;

  /// The step number (as per Abaqus definition) can be set by the user
  int _aqKSTEP;

  /// Increment number (_t_step). This can be supported. The step number (as per Abaqus definition)
  /// is not supported by MOOSE.
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
  std::vector<Real> _aqDROT;

  /// User-defined number of material constants associated with this user material.
  int _aqNPROPS;

  /// Array of interpolated values of predefined field variables at this point at the start of the increment, based on the values read in at the nodes.
  std::vector<Real> _aqPREDEF;

  /// Array of increments of predefined field variables
  std::vector<Real> _aqDPRED;

  void initQpStatefulProperties() override;
  void computeQpStress() override;

  const MaterialProperty<RankTwoTensor> & _stress_old;
  const MaterialProperty<RankTwoTensor> & _total_strain_old;
  const OptionalMaterialProperty<RankTwoTensor> & _strain_increment;

  /// Jacobian multiplier
  MaterialProperty<RankFourTensor> & _jacobian_mult;

  const OptionalMaterialProperty<RankTwoTensor> & _Fbar;
  const OptionalMaterialProperty<RankTwoTensor> & _Fbar_old;

  MaterialProperty<std::vector<Real>> & _state_var;
  const MaterialProperty<std::vector<Real>> & _state_var_old;

  MaterialProperty<Real> & _elastic_strain_energy;
  MaterialProperty<Real> & _plastic_dissipation;
  MaterialProperty<Real> & _creep_dissipation;

  /// recommended maximum timestep for this model under the current conditions
  MaterialProperty<Real> & _material_timestep;

  // Time step rotation increment
  const OptionalMaterialProperty<RankTwoTensor> & _rotation_increment;
  const OptionalMaterialProperty<RankTwoTensor> & _rotation_increment_old;

  // Coupled temperature field
  const VariableValue & _temperature;
  const VariableValue & _temperature_old;

  // Coupled user-defined field
  const std::vector<const VariableValue *> _external_fields;
  const std::vector<const VariableValue *> _external_fields_old;

  // External field names
  std::vector<VariableName> _external_field_names;

  // Number of external fields provided by the user
  const std::size_t _number_external_fields;

  const std::vector<MaterialPropertyName> _external_property_names;
  const std::size_t _number_external_properties;
  std::vector<const MaterialProperty<Real> *> _external_properties;
  std::vector<const MaterialProperty<Real> *> _external_properties_old;

  /// parameter to assist with the transition to 1-based indexing
  const bool _use_one_based_indexing;

  /// Rotation information
  const bool _use_orientation;
  const RotationTensor _R;
  MaterialProperty<RankTwoTensor> & _total_rotation;
  const MaterialProperty<RankTwoTensor> & _total_rotation_old;

private:
  /// Method being used to compute strain and rotation increments
  const ComputeFiniteStrain::DecompMethod _decomposition_method;

  /// User object that determines step number
  const StepUserObject * _step_user_object;
};

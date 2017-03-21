/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef INTERACTIONINTEGRALAUXFIELDS_H
#define INTERACTIONINTEGRALAUXFIELDS_H

#include "Material.h"
#include "MaterialProperty.h"
#include "CrackFrontDefinition.h"
#include "SymmTensor.h"
#include "LineSegment.h"

// Forward Declarations
class InteractionIntegralAuxFields;

template <>
InputParameters validParams<InteractionIntegralAuxFields>();
void addInteractionIntegralAuxFieldsParams(InputParameters & params);

/**
 * Auxiliary stress and displacement fields from Williams'
 * stress intensity factor solutions for use in interaction
 * integral calculation.
 */
class InteractionIntegralAuxFields : public Material
{
public:
  InteractionIntegralAuxFields(const InputParameters & parameters);

  static std::vector<MooseEnum> getSIFModesVec(unsigned int n);
  static MooseEnum getSIFModesEnum();

protected:
  virtual void computeQpProperties();

  std::string _appended_index_name;
  MaterialProperty<ColumnMajorMatrix> & _aux_stress_I;
  MaterialProperty<ColumnMajorMatrix> & _aux_disp_I;
  MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp_I;
  MaterialProperty<ColumnMajorMatrix> & _aux_strain_I;
  MaterialProperty<ColumnMajorMatrix> & _aux_stress_II;
  MaterialProperty<ColumnMajorMatrix> & _aux_disp_II;
  MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp_II;
  MaterialProperty<ColumnMajorMatrix> & _aux_strain_II;
  MaterialProperty<ColumnMajorMatrix> & _aux_stress_III;
  MaterialProperty<ColumnMajorMatrix> & _aux_disp_III;
  MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp_III;
  MaterialProperty<ColumnMajorMatrix> & _aux_strain_III;
  MaterialProperty<ColumnMajorMatrix> & _aux_stress_T;
  MaterialProperty<ColumnMajorMatrix> & _aux_disp_T;
  MaterialProperty<ColumnMajorMatrix> & _aux_grad_disp_T;
  MaterialProperty<ColumnMajorMatrix> & _aux_strain_T;

  enum SIF_MODE
  {
    KI,
    KII,
    KIII,
    T
  };

  void computeAuxFields(const SIF_MODE sif_mode,
                        ColumnMajorMatrix & stress,
                        ColumnMajorMatrix & disp,
                        ColumnMajorMatrix & grad_disp,
                        ColumnMajorMatrix & strain);
  void computeTFields(ColumnMajorMatrix & stress, ColumnMajorMatrix & grad_disp);

private:
  const CrackFrontDefinition * _crack_front_definition;
  const unsigned int _crack_front_point_index;
  std::vector<SIF_MODE> _sif_mode;
  Real _poissons_ratio;
  Real _youngs_modulus;
  Real _kappa;
  Real _shear_modulus;
  Real _r;
  Real _theta;
};

#endif // INTERACTIONINTEGRALAUXFIELDS_H

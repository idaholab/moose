/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "SolidModel.h"

#ifndef ABAQUSUMATMATERIAL_H
#define ABAQUSUMATMATERIAL_H

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
                       Real * CMNAME,
                       int * NDI,
                       int * NSHR,
                       int * NTENS,
                       int * NSTATV,
                       Real PROPS[],
                       int * NPROPS,
                       Real COORDS[],
                       Real DROT[][3],
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

// Forward Declaration
class AbaqusUmatMaterial;

template <>
InputParameters validParams<AbaqusUmatMaterial>();

// AbaqusUmatMateral class define a property
class AbaqusUmatMaterial : public SolidModel
{
public:
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
  Real _SSE, _SPD, _SCD, _DRPLDT, _RPL, _PNEWDT, _DTIME, _TEMP, _DTEMP, _CMNAME, _CELENT;

  // UMAT integer values
  int _NDI, _NSHR, _NTENS, _NSTATV, _NPROPS, _NOEL, _NPT, _LAYER, _KSPT, _KSTEP, _KINC;

  // UMAT arrays
  Real *_STATEV, *_DDSDDT, *_DRPLDE, *_STRAN, _PREDEF[1], _DPRED[1], _COORDS[3], _DROT[3][3],
      *_DFGRD0, *_DFGRD1, *_STRESS, *_DDSDDE, *_DSTRAN, _TIME[2], *_PROPS;

  virtual void initQpStatefulProperties();
  virtual void computeStress();

  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;
  const VariableGradient & _grad_disp_x_old;
  const VariableGradient & _grad_disp_y_old;
  const VariableGradient & _grad_disp_z_old;
  MaterialProperty<std::vector<Real>> & _state_var;
  MaterialProperty<std::vector<Real>> & _state_var_old;
  MaterialProperty<ColumnMajorMatrix> & _Fbar;
  MaterialProperty<ColumnMajorMatrix> & _Fbar_old;
  MaterialProperty<Real> & _elastic_strain_energy;
  MaterialProperty<Real> & _plastic_dissipation;
  MaterialProperty<Real> & _creep_dissipation;
};

#endif // ABAQUSUMATMATERIAL_H

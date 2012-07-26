#include "SolidModel.h"

#ifndef ABAQUSUMATMATERIAL_H
#define ABAQUSUMATMATERIAL_H


typedef void (*umat_t)(Real STRESS[], Real STATEV[], Real DDSDDE[][6], Real* SSE, Real* SPD, Real* SCD, Real* RPL, Real DDSDDT[], Real DRPLDE[], Real* DRPLDT, Real STRAN[], Real DSTRAN[], Real TIME[], Real* DTIME, Real* TEMP, Real* DTEMP, Real PREDEF[], Real DPRED[], Real* CMNAME, int* NDI, int*NSHR, int*NTENS, int* NSTATV, Real PROPS[], int* NPROPS, Real COORDS[], Real DROT[][3], Real* PNEWDT, Real* CELENT, Real DFGRD0[][3], Real DFGRD1[][3], int* NOEL, int* NPT, int* LAYER, int* KSPT, int* KSTEP, int* KINC);


//Forward Declaration
class AbaqusUmatMaterial;

template<>
InputParameters validParams<AbaqusUmatMaterial>();

//Plastic Fortran Interface class define a property
class AbaqusUmatMaterial : public SolidModel
{
public:
  AbaqusUmatMaterial(const std::string & name,
                  InputParameters parameters);
  virtual ~AbaqusUmatMaterial();
  
    
protected:
  FileName _plugin;
  std::vector<Real> _mechanical_constants;
  std::vector<Real> _thermal_constants;
  unsigned int _num_state_vars;
  unsigned int _num_props;

  /// The plugin library handle
  void * _handle;

  /// Function pointer to the dynamically loaded function
  umat_t _umat;

  //Dimension real scalar values
  Real  _SSE, _SPD, _SCD, _DRPLDT, _RPL, _PNEWDT, _DTIME, _TEMP, _DTEMP, _CMNAME, _CELENT;

  //Dimension integer values
  int  _NDI, _NSHR, _NTENS, _NSTATV, _NPROPS, _NOEL, _NPT, _LAYER, _KSPT, _KSTEP, _KINC, i;

  //Dimension arrays from FORTRAN file
  Real * _STATEV,  * _DDSDDT, * _DRPLDE, * _STRAN, _PREDEF[1], _DPRED[1], _COORDS[3], _DROT[3][3], _DFGRD0[3][3], _DFGRD1[3][3], _STRESS[6], _DDSDDE[6][6], _DSTRAN[6], _TIME[2], * _PROPS;
  
  //Dimension arrays to be used as references from FORTRAN arrays
  Real mySTRESS[6];

  virtual void initQpStatefulProperties();
  virtual void computeStress();

  MaterialProperty<std::vector<Real> > & _state_var;
  MaterialProperty<std::vector<Real> > & _state_var_old;  
};

#endif //ABAQUSUMATMATERIAL_H

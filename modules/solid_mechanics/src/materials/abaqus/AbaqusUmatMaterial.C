#include "AbaqusUmatMaterial.h"

#include <dlfcn.h>



template<>
InputParameters validParams<AbaqusUmatMaterial>()
{
  InputParameters params = validParams<SolidModel>();
  params.addRequiredParam<FileName>("plugin","The path to the compiled dynamic library for the plugin you want to use");
  params.addRequiredParam<std::vector<Real> >("mechanical_constants", "Mechanical Material Properties");
  params.addParam<std::vector<Real> >("thermal_constants", "Thermal Material Properties");
  return params;
}

AbaqusUmatMaterial::AbaqusUmatMaterial(const std::string  & name,
                                 InputParameters parameters) :
    SolidModel( name, parameters ),
    _plugin(getParam<FileName>("plugin")),
    _mechanical_constants(getParam<std::vector<Real> >("mechanical_constants")),
    _thermal_constants(getParam<std::vector<Real> >("thermal_constants")),
    _pstrain(declareProperty<Real>("pstrain")),
    _pstrain_old(declarePropertyOld<Real>("pstrain")),
    _hardeningvariable(declareProperty<Real>("hardeningvariable")),
    _hardeningvariable_old(declarePropertyOld<Real>("hardeningvariable"))
{
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
  _umat = (umat_t) dlsym(_handle, "umat_");

  // Catch errors
  const char *dlsym_error = dlerror();
  if (dlsym_error)
  {
    dlclose(_handle);
    std::ostringstream error;
    error << "Cannot load symbol 'hello': " << dlsym_error << '\n';
    mooseError(error.str());
  }    
}

AbaqusUmatMaterial::~AbaqusUmatMaterial()
{
  dlclose(_handle);
}

void AbaqusUmatMaterial::computeStress()
{
  //Read mesh dimension and size UMAT arrays
  if (_mesh.dimension()==3)  //3D case
  { 
    _NTENS=6;  //Size of the stress or strain component array (NDI+NSHR)
    _NSHR=3;   //Number of engineering shear stress components
    _NDI=3;    //Number of direct stress components (always 3)
  }
  else  //2D case
  {
    _NTENS=4;
    _NSHR=1;
    _NDI=3;
  }

  //Assign materials properties from vector form into an array
  //Vector form for LSH Plastic UMAT : 'youngsmodulus poissonsratio yieldstress hardeningslope'
  for (i=0; i<_mechanical_constants.size(); i++)
    _PROPS[i] = _mechanical_constants[i];
  
  //Dimension UMAT state variable (NSTATV) and material constant (NPROPS) arrays
  _NSTATV = 3;
  _NPROPS = 4;
  
  //Recover "old" state variables to input into UMAT
  _STATEV[0]= _pstrain_old[_qp];
  _STATEV[1]= _hardeningvariable_old[_qp];
  
  //Initialize/Update stress array values
  for (i=0; i<_NTENS; i++)
    _STRESS[i] = _stress_old.component(i);

  //Pass through total strain increment array
  for (i=0; i<_NTENS; i++)
    _DSTRAN[i] = _strain_increment.component(i);
  
  //Pass through step and time information
  _KSTEP = _t_step;  //Step number
  _TIME[0] = _dt;    //Value of step time at the beginning of the current increment
  _TIME[1] = _t;     //Value of total time at the beginning of the current increment
  _DTIME = _dt;      //Time increment

  //Connection to extern statement
  _umat(_STRESS, _STATEV, _DDSDDE, &_SSE, &_SPD, &_SCD, &_RPL, _DDSDDT, _DRPLDE, &_DRPLDT, _STRAN, _DSTRAN, _TIME, &_DTIME, &_TEMP, &_DTEMP, _PREDEF, _DPRED, &_CMNAME, &_NDI, &_NSHR, &_NTENS, &_NSTATV, _PROPS, &_NPROPS, _COORDS, _DROT, &_PNEWDT, &_CELENT, _DFGRD0, _DFGRD1, &_NOEL, &_NPT, &_LAYER, &_KSPT, &_KSTEP, &_KINC);

  //Update state variables
  _pstrain[_qp]=_STATEV[0];
  _hardeningvariable[_qp]=_STATEV[1];

  // Update stress
  for (i=0; i<_NTENS; i++)
    mySTRESS[i]=_STRESS[i];

  SymmTensor stressnew(mySTRESS[0], mySTRESS[1], mySTRESS[2], mySTRESS[3], mySTRESS[4], mySTRESS[5]);
  _stress[_qp] = stressnew;
  _stress[_qp] += _stress_old;
}

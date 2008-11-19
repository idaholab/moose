#include "PorousMedia.h"

template <class T>
const T& min ( const T& a, const T& b )
{
  return (a<b)?a:b;
}

template<>
Parameters valid_params<PorousMedia>()
{
  Parameters params;
  params.set<Real>("thermal_conductivity")=1.0;
  params.set<Real>("thermal_conductivity_fluid")=0.3707; //[W/(m.K)]
  params.set<Real>("thermal_conductivity_solid")=80.0;   //[W/(m.K)]
  params.set<Real>("specific_heat")=1.0;
  params.set<Real>("specific_heat_fluid")=5195;  //[J/(kg.K)]
  params.set<Real>("specific_heat_solid")=1725;
  params.set<Real>("neutron_diffusion_coefficient")=1.0;
  params.set<Real>("neutron_absorption_xs")=1.0;
  params.set<Real>("neutron_fission_xs")=1.0;
  params.set<Real>("neutron_per_fission")=1.0;
  params.set<Real>("neutron_velocity")=1.0;
  params.set<Real>("neutron_per_power")=1.0;
  params.set<Real>("heat_xfer_coefficient")=1.0;
  params.set<Real>("temp0")=1.0;
  params.set<Real>("k0")=1.0;
  params.set<Real>("k1")=0.0;
  params.set<Real>("d0")=1.0;
  params.set<Real>("d1")=0.0;
  params.set<Real>("siga0")=1.0;
  params.set<Real>("siga1")=0.0;
  params.set<Real>("sigf0")=1.0;
  params.set<Real>("sigf1")=0.0;
  params.set<Real>("fluid_resistance_coefficient")=1.0;
  params.set<Real>("gas_constant")=1.0;
  params.set<Real>("porosity")=0.5;
  // parameter of pebble-bed reactor
  params.set<Real>("vessel_cross_section")=1.0;
  params.set<Real>("pebble_diameter")=0.06;
  params.set<bool>("kta_standard")=false;
  params.set<bool>("non_dim_flag")=false;

  return params;
}

void
PorousMedia::computeProperties()
{
  for(unsigned int qp=0; qp<_qrule->n_points(); qp++)
  {
    Real  temp_diff = _solid_temp[qp]-_my_temp0;
    
    _thermal_conductivity[qp]          = _my_thermal_conductivity;
    _thermal_conductivity_fluid[qp]    = _my_thermal_conductivity_fluid;
    _thermal_conductivity_solid[qp]    = _my_thermal_conductivity_solid;
    _specific_heat[qp]                 = _my_specific_heat;
    _specific_heat_fluid[qp]           = _my_specific_heat_fluid;
    _specific_heat_solid[qp]           = _my_specific_heat_solid;
    _heat_xfer_coefficient[qp]         = _my_heat_xfer_coefficient;
    _fluid_resistance_coefficient[qp]  = _my_fluid_resistance_coefficient;
    _gas_constant[qp]                  = _my_gas_constant;
    _porosity[qp]                      = _my_porosity;
    _neutron_diffusion_coefficient[qp] = _my_d0+_my_d1*temp_diff;
    _neutron_absorption_xs[qp]         = _my_siga0+_my_siga1*temp_diff;
    _neutron_fission_xs[qp]            = _my_sigf0+_my_sigf1*temp_diff;
    _neutron_per_power[qp]             = _my_neutron_per_power;
    _neutron_per_fission[qp]           = _my_neutron_per_fission;
    _neutron_velocity[qp]              = _my_neutron_velocity;

//KTA standard
    
    if( _my_kta_standard )
    {
      /*
      //porosity
      static Real porosity_inf = 0.41;
      static Real porosity_max = 0.9;

      //dimension of the pebble bed
      static Real r_max = +0.75;
      static Real r_min = +0.07;
      static Real z_max = +0.5;
      static Real z_min = -0.5;

      
      Real dist_r = min( _q_point[qp](0) - r_min , r_max - _q_point[qp](0) );
      Real dist_z = min( _q_point[qp](1) - z_min , z_max - _q_point[qp](1) );

      Real C = 100;
      Real Y = min(dist_r,dist_z);
      
      Real _porosity[qp] = porosity_inf * (1 + (porosity_max - porosity_inf)/porosity_inf * exp(-C*Y));
      */
      
      //fluid thermal conductivity
      Real pre_in_bar = 1.0;
      if( _has_pre)
        pre_in_bar = _pre[qp]/1e5;
      
      _thermal_conductivity_fluid[qp] = 2.682e-3*(1+1.123e-3*pre_in_bar)*pow((_fluid_temp[qp]+_solid_temp[qp])/2,0.71)*(1-2e-4*pre_in_bar);

      //Solid
      Real temp_in_c = _solid_temp[qp]-273.5;
      Real kappa_pebble = 186.021-39.5408e-2*temp_in_c+4.8852e-4*pow(temp_in_c,2)-2.91e-7*pow(temp_in_c,3)+6.6e-11*pow(temp_in_c,4);
      Real b = 1.25*pow((1-_porosity[qp])/_porosity[qp],1.111);
      static Real sigma = 5.67e-8;
      static Real emissivity = 0.8;
    
      Real lambda = kappa_pebble/(4*sigma*pow(_solid_temp[qp],3)*_my_pebble_diameter);

      //void radiation+solid conduction
      Real a1 = (1-pow(1-_porosity[qp],0.5))*_porosity[qp];
      Real a2 = pow(1-_porosity[qp],0.5)/(2/emissivity-1);
      Real a3 = (b+1)/b;
      Real a4 = 1/(1+1/((2/emissivity-1)*lambda));
      Real lambda_r = 4*sigma*pow(_solid_temp[qp],3)*_my_pebble_diameter*(a1+a2*a3*a4);

      //gas conduction+solid conduction
      Real b1 = pow(1-_porosity[qp],0.5);
      Real ratio_lambda = _thermal_conductivity_fluid[qp]/kappa_pebble;
  
      Real b2 = 2*b1/(1-ratio_lambda*b);
      Real b3 = (1-ratio_lambda)*b/pow(1-ratio_lambda*b,2)*log(1/(ratio_lambda*b));
      Real b4 = (b+1)/2;
      Real b5 = (b-1)/(1-ratio_lambda*b);
      Real lambda_g = (1-b1+b2*(b3-b4-b5))*_thermal_conductivity_fluid[qp];
    
      //contact conduction+solid conduction
      static Real s  = 1;
      static Real sf = 1;
      Real na = 1/(pow(_my_pebble_diameter,2));
      Real nl = 1/(_my_pebble_diameter);
      static Real poisson_ratio_pebble = 0.136;
      static Real young_modules_pebble = 9e9;
      Real f = 101325.0*sf/na;
      if( _has_pre )
        f = _pre[qp]*sf/na;
      
      Real c1 = 3*(1-poisson_ratio_pebble*poisson_ratio_pebble)*f*0.03/4/young_modules_pebble;
      Real c2 = pow(c1,0.333);
      Real lambda_c = c2*na/nl/(0.531*s)*kappa_pebble;
      _thermal_conductivity_solid[qp] = lambda_r+lambda_g+lambda_c;
      
      //Fluid
      Real density = 101325/(_my_gas_constant*_fluid_temp[qp]);
      if( _has_pre)
        density = _pre[qp]/(_my_gas_constant*_fluid_temp[qp]);
      Real mom = 0;
      if( _has_xmom)
      {
        mom    = _xmom[qp]*_xmom[qp];
        if( _dim >=2 )
          mom +=_ymom[qp]*_ymom[qp];
        if( _dim == 3)
          mom +=_zmom[qp]*_zmom[qp];
        mom = pow(mom,0.5);
      }
      if( _has_rmom)
      {
        mom    = _rmom[qp]*_rmom[qp];
        if( _dim >=2 )
          mom +=_zmom[qp]*_zmom[qp];
        if( _dim == 3)
          mom +=_thetamom[qp]*_thetamom[qp];
        mom = pow(mom,0.5);
      }
      mom = mom*_porosity[qp];
      
      Real dyn_viscosity = 3.674e-7*pow((_fluid_temp[qp]+_solid_temp[qp])/2,0.7);
      Real reynolds      = mom*_my_pebble_diameter/dyn_viscosity;
      
      Real prandtl       = _specific_heat_fluid[qp]*dyn_viscosity/_thermal_conductivity_fluid[qp];

      Real nusselt = 2;
      nusselt = (7-10*_porosity[qp]+5*pow(_porosity[qp],2))*(1+0.7*pow(reynolds,0.2)*pow(prandtl,0.333))
                     +(1.33-2.4*_porosity[qp]+1.2*pow(_porosity[qp],2))*pow(reynolds,0.7)*pow(prandtl,0.333);
      
      Real area = 6*(1-_porosity[qp])/_my_pebble_diameter;

      _heat_xfer_coefficient[qp] = area*nusselt*_thermal_conductivity_fluid[qp]/_my_pebble_diameter;
      
      if( _has_rmom || _has_xmom )
      {
        Real reynolds_over_1meps = reynolds/(1-_porosity[qp]);

        Real w1 = 320*dyn_viscosity*(1-_porosity[qp])/pow(_my_pebble_diameter,2)/density/2;
        Real w2 = 0;
        if( mom >0)
          w2 = 6*mom/_my_pebble_diameter/2/density/pow(reynolds_over_1meps,0.1);
        _fluid_resistance_coefficient[qp] = w1+w2;
        //std::cout <<"w: " << _fluid_resistance_coefficient[qp] << " " << reynolds_over_1meps<< std::endl;
        
        /*
        _fluid_resistance_coefficient[qp] =163*dyn_viscosity*pow(1-_porosity[qp],2)/pow(_my_pebble_diameter,2)/density/pow(_porosity[qp],2);
        if( reynolds_over_1meps > 1 )
        {  
          Real psi = 320/reynolds_over_1meps+6/pow(reynolds_over_1meps,0.1);
          _fluid_resistance_coefficient[qp] = psi*(1-_porosity[qp])/pow(_porosity[qp],3)/_my_pebble_diameter/2/density*mom;
        }
        */
      }
    }
  }
}

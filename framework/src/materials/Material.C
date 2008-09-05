#include "Material.h"

void
Material::materialReinit()
{
  _zero.resize(_qrule->n_points(),0);
  _grad_zero.resize(_qrule->n_points(),0);
  _thermal_conductivity.resize(_qrule->n_points(),1);
  _thermal_expansion.resize(_qrule->n_points(),1);
  _specific_heat.resize(_qrule->n_points(),1);
  _density.resize(_qrule->n_points(),1);
  _youngs_modulus.resize(_qrule->n_points(),1);
  _poissons_ratio.resize(_qrule->n_points(),1);
  _neutron_diffusion_coefficient.resize(_qrule->n_points(),1);
  _neutron_absorption_xs.resize(_qrule->n_points(),1);
  _neutron_fission_xs.resize(_qrule->n_points(),1);
  _neutron_per_fission.resize(_qrule->n_points(),1);

  computeProperties();
}

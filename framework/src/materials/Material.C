#include "Material.h"

void
Material::materialReinit()
{
  _thermal_conductivity.resize(_qrule->n_points(),1);
  _thermal_expansion.resize(_qrule->n_points(),1);
  _specific_heat.resize(_qrule->n_points(),1);
  _density.resize(_qrule->n_points(),1);
  _youngs_modulus.resize(_qrule->n_points(),1);
  _poissons_ratio.resize(_qrule->n_points(),1);

  computeProperties();
}

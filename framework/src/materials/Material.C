#include "Material.h"
#include <iostream>

void
Material::materialReinit()
{
  std::map<std::string, std::vector<Real> >::iterator it = _real_props.begin();
  std::map<std::string, std::vector<Real> >::iterator it_end = _real_props.end();

  for(;it!=it_end;++it)
    it->second.resize(_qrule->n_points(),1);

  computeProperties();
}

/**
 * Updates the old (first) material properties to the current/new material properies (second)
 */
void
Material::updateDataState()
{
  std::map<unsigned int, std::pair<QpData *, QpData *> >::iterator it = _qp_props.begin();
  std::map<unsigned int, std::pair<QpData *, QpData *> >::iterator it_end = _qp_props.end();

  std::cerr << "Updating QpData\n";
  for(;it!=it_end;++it)
    *(it->second.first) = *(it->second.second);
}

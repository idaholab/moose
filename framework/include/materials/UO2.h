#include "Material.h"

#ifndef UO2_H
#define UO2_H

/**
 * Simple Uranium Oxide material.
 */
class UO2 : public Material
{
public:
  UO2(std::string name,
      Parameters parameters,
      unsigned int block_id,
      std::vector<std::string> coupled_to,
      std::vector<std::string> coupled_as)
    :Material(name,parameters,block_id,coupled_to,coupled_as)
  {}

protected:

  virtual void computeProperties();
};

#endif //UO2_H

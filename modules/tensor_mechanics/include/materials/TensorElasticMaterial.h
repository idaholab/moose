// Original class author: A.M. Jokisaari,  O. Heinonen

#ifndef TENSORELASTICMATERIAL_H
#define TENSORELASTICMATERIAL_H

#include "TensorMechanicsMaterial.h"

/**
 * TensorElasticMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs, or only 9, depending on the
 * boolean input value given.  This can be extended or simplified to specify HCP, monoclinic,
 * cubic, etc as needed.
 */

class TensorElasticMaterial : public TensorMechanicsMaterial
{
public:
  TensorElasticMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpStress();
  
private:

};

#endif //LINEARELASTICMATERIAL_H

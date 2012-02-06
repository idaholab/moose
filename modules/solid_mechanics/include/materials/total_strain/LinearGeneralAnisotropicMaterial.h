// Original class author: A.M. Jokisaari

#ifndef LINEARGENERALANISOTROPICMATERIAL_H
#define LINEARGENERALANISOTROPICMATERIAL_H

#include "SolidMechanicsMaterial.h"
#include "SymmTensor.h"
#include "SymmAnisotropicElasticityTensor.h"

/**
 * LinearGeneralAnisotropicMaterial handles a fully anisotropic, single-crystal material's elastic
 * constants.  It takes all 21 independent stiffness tensor inputs.  This can be extended or
 * simplified to specify HCP, monoclinic, cubic, etc as needed.
 */

//Forward declaration
class LinearGeneralAnisotropicMaterial;

template<>
InputParameters validParams<LinearGeneralAnisotropicMaterial>();

class LinearGeneralAnisotropicMaterial : public SolidMechanicsMaterial
{
public:
  LinearGeneralAnisotropicMaterial(const std:: string & name, InputParameters parameters);

protected:
  virtual void computeQpProperties();
  
private:
  // vectors to get the input values
  std::vector<Real> _Cijkl_matrix_vector;
  
  // bool to indicate if using 9 stiffness values or all 21
  bool _all_21;

  // Pointers for the individual material information
  SymmElasticityTensor _Cijkl_matrix;
  MaterialProperty<SymmElasticityTensor > &_Cijkl_matrix_MP;
  
};

#endif //LINEARGENERALANISOTROPICMATERIAL_H

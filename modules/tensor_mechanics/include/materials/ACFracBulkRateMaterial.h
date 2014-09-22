#ifndef ACFRACBULKRATEMATERIAL_H
#define ACFRACBULKRATEMATERIAL_H

#include "Material.h"
#include "Function.h"

class ACFracBulkRateMaterial;

template<>
InputParameters validParams<ACFracBulkRateMaterial>();

class ACFracBulkRateMaterial : public Material
{
public:
  ACFracBulkRateMaterial(const std::string & name,
                        InputParameters parameters);

protected:
  virtual void computeQpProperties();
  virtual void initQpStatefulProperties();
  virtual void read_prop();
  virtual void func_prop();

  MaterialProperty<Real> & _L;

  MaterialProperty<Real> &_gc_prop_tens;
  MaterialProperty<Real> &_gc_prop_tens_old;


  bool _has_function;
  Function * _function;

  Real _L0;
  Real _gc_tens;

  std::string _frac_prop_file_name;

private:


};

#endif //ACFRACBULKRATEMATERIAL_H

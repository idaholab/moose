#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"

class MaterialTensorAux;

template<>
InputParameters validParams<MaterialTensorAux>();


class MaterialTensorAux : public AuxKernel
{
public:
  MaterialTensorAux( const std::string & name, InputParameters parameters );

  virtual ~MaterialTensorAux() {}

protected:
  enum MTA_ENUM
  {
    MTA_COMPONENT,
    MTA_VONMISES,
    MTA_HYDROSTATIC,
    MTA_FIRSTINVARIANT,
    MTA_SECONDINVARIANT,
    MTA_THIRDINVARIANT
  };


  virtual Real computeValue();

  MaterialProperty<RealTensorValue> & _tensor;
  const int _index;
  std::string _quantity_string;
  MTA_ENUM _quantity;

};

#endif // MATERIALTENSORAUX_H

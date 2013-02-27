#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"

class MaterialTensorAux;
class SymmTensor;

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
    MTA_PLASTICSTRAINMAG,
    MTA_HYDROSTATIC,
    MTA_HOOP,
    MTA_RADIAL,
    MTA_AXIAL,
    MTA_FIRSTINVARIANT,
    MTA_SECONDINVARIANT,
    MTA_THIRDINVARIANT,
    MTA_TRIAXIALITY
  };


  virtual Real computeValue();

  MaterialProperty<SymmTensor> & _tensor;
  const int _index;
  MooseEnum _quantity_moose_enum;
  MTA_ENUM _quantity;

  const Point _p1;
  const Point _p2;

};

#endif // MATERIALTENSORAUX_H

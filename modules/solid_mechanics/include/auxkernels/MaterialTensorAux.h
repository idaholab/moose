#ifndef MATERIALTENSORAUX_H
#define MATERIALTENSORAUX_H

#include "AuxKernel.h"

class MaterialTensorAux;
class SymmTensor;

enum MTA_ENUM
{
  MTA_COMPONENT,
  MTA_VONMISES,
  MTA_PLASTICSTRAINMAG,
  MTA_HYDROSTATIC,
  MTA_HOOP,
  MTA_RADIAL,
  MTA_AXIAL,
  MTA_MAXPRINCIPAL,
  MTA_MEDPRINCIPAL,
  MTA_MINPRINCIPAL,
  MTA_FIRSTINVARIANT,
  MTA_SECONDINVARIANT,
  MTA_THIRDINVARIANT,
  MTA_TRIAXIALITY,
  MTA_VOLUMETRICSTRAIN
};

template<>
InputParameters validParams<MaterialTensorAux>();
void addMaterialTensorParams(InputParameters& params);

class MaterialTensorAux : public AuxKernel
{
public:
  MaterialTensorAux( const std::string & name, InputParameters parameters );

  virtual ~MaterialTensorAux() {}

  static void checkMaterialTensorParams(MTA_ENUM & quantity,
                                        const MooseEnum & quantity_moose_enum,
                                        const int index,
                                        const std::string & name);

  static Real getTensorQuantity(const SymmTensor & tensor,
                                const MTA_ENUM quantity,
                                const MooseEnum & quantity_moose_enum,
                                const int index,
                                const Point * curr_point,
                                const Point * p1,
                                const Point * p2);

protected:

  virtual Real computeValue();

  static Real principalValue( const SymmTensor & tensor, unsigned int index );

  MaterialProperty<SymmTensor> & _tensor;
  const int _index;
  MooseEnum _quantity_moose_enum;
  MTA_ENUM _quantity;

  const Point _p1;
  const Point _p2;

};

#endif // MATERIALTENSORAUX_H

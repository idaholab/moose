#include "InterfaceKernel.h"

class HeatConvectionInterface;

/**
 * Interface kernel for coupling two variables with convective heat transfer
 * such that the heat flux \f$h(u_1-u_2)\f$ matches in both domains, where
 * \f$h\f$ is the convective heat transfer coefficient, \f$u_1\f$ is the
 * variable defined on the present block and \f$u_2\f$ is the variable
 * defined on the adjacent block. To obtain the correct sign, \f$u_1\f$ must
 * correspond to the variable that is on the "inside" of the boundary on
 * which this kernel is applied. In other words, when this interface kernel
 * is applied to the boundary in the following two-block domain,
 *
 *  -----------|-----------
 *  |          |--> n     |
 *  |    u_1   |    u_2   |
 *  -----------------------
 *
 * the unit normal for the boundary connecting the two variables must point
 * _from_ \f$u_1\f$'s block _to_ \f$u_2\f$'s block.
 */
class HeatConvectionInterface : public InterfaceKernel
{
public:
  static InputParameters validParams();

  HeatConvectionInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  /// Convective heat transfer coefficient
  const ADMaterialProperty<Real> & _h;
};

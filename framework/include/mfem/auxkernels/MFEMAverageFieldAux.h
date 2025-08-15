#pragma once

#ifdef MFEM_ENABLED
n
#include "MFEMAuxKernel.h"
#include "mfem.hpp"

    /**
     * AuxKernel to compute a time-weighted running average of MFEMVariable
     * via an mfem::Coefficient and ProjectCoefficient().
     *
     *   avg_new(x) = (1 - w)*avg_old(x) + w*src(x), w = dt / (t - skip)
     */
    class MFEMAverageFieldAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMAverageFieldAux(const InputParameters & parameters);

  virtual ~MFEMAverageFieldAux() = default;

  virtual void execute() override;

protected:
  /// Name of the source MFEMVariable name to take the average from
  const VariableName _source_var_name;
  /// Reference to the MFEMVariable underlying GridFunction
  const mfem::ParGridFunction & _source_var;
  /// Solver time step size
  const mfem::real_t _dt;
  /// Time before the averaging starts
  const mfem::real_t _skip;

  /**
   * a derived class from mfem::Coefficient to inherit the mfem::Coefficient eval function ,
   *  which reads the old average saved in the AuxVariable and new field value from the source
   * gridfunction and returns: (1 - w)*previous_average + w * current_field with w = dt/(t - skip).
   */
  class AvgCoef : public mfem::Coefficient
  {
  public:
    AvgCoef(mfem::ParGridFunction & avg_gf,
            const mfem::ParGridFunction & src_gf,
            mfem::real_t time,
            mfem::real_t dt,
            mfem::real_t skip)
      : _avg_gf(avg_gf), _src_gf(src_gf), _time_ref(time), _dt(dt), _skip(skip)
    {
    }
    virtual ~AvgCoef();
    virtual double Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override
    {
      const double old_v = _avg_gf.GetValue(T, ip);
      const double new_v = _src_gf.GetValue(T, ip);
      const double w = (_time_ref > _skip) ? (_dt / (_time_ref - _skip)) : 0.0;
      return (1.0 - w) * old_v + w * new_v;
    }

  private:
    mfem::ParGridFunction & _avg_gf;
    const mfem::ParGridFunction & _src_gf;
    mfem::real_t _time_ref;
    const mfem::real_t _dt;
    const mfem::real_t _skip;
  };
};

#endif

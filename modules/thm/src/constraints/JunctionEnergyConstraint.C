#include "JunctionEnergyConstraint.h"
#include "JunctionStagnationEnthalpyUserObject.h"
#include "Assembly.h"
#include "SystemBase.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "SinglePhaseFluidProperties.h"
#include "Numerics.h"

registerMooseObject("THMApp", JunctionEnergyConstraint);

template <>
InputParameters
validParams<JunctionEnergyConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();

  params.addRequiredParam<std::vector<Real>>("normals", "node normals");
  params.addRequiredParam<std::vector<dof_id_type>>("nodes", "node IDs");
  params.addRequiredParam<std::vector<Real>>("K",
                                             "Loss coefficients for each flow channel at junction");

  params.addRequiredCoupledVar("A", "Cross-sectional area");
  params.addRequiredCoupledVar("rhoA", "rho*A");
  params.addRequiredCoupledVar("rhouA", "rho*u*A");
  params.addRequiredCoupledVar("rhoEA", "rho*E*A");
  params.addRequiredCoupledVar("s_junction", "Junction entropy variable");

  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("v", "Specific volume");
  params.addRequiredCoupledVar("e", "Specific internal energy");
  params.addRequiredCoupledVar("p", "Pressure");

  params.addRequiredParam<UserObjectName>("H_junction_uo",
                                          "Junction stagnation enthalpy user object");

  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties user object to use.");

  return params;
}

JunctionEnergyConstraint::JunctionEnergyConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters),

    _normals(getParam<std::vector<Real>>("normals")),
    _K(getParam<std::vector<Real>>("K")),

    _A(coupledValue("A")),
    _rhoA(coupledValue("rhoA")),
    _rhouA(coupledValue("rhouA")),
    _rhouA_old(coupledValueOld("rhouA")),
    _rhoEA(coupledValue("rhoEA")),
    _s_junction(coupledScalarValue("s_junction")),

    _rhoA_var_number(coupled("rhoA")),
    _rhouA_var_number(coupled("rhouA")),
    _rhoEA_var_number(coupled("rhoEA")),
    _s_junction_var_number(coupledScalar("s_junction")),

    _vel(coupledValue("vel")),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _p(coupledValue("p")),

    _H_junction_uo(getUserObject<JunctionStagnationEnthalpyUserObject>("H_junction_uo")),

    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
  _master_node_vector = getParam<std::vector<dof_id_type>>("nodes");
  // just a dummy value that is never used
  _connected_nodes.push_back(*_master_node_vector.begin());
}

void
JunctionEnergyConstraint::computeResidual(NumericVector<Number> & /*residual*/)
{
  auto && dofs = _var.dofIndices();
  DenseVector<Number> re(dofs.size());

  re.zero();
  for (unsigned int i = 0; i < dofs.size(); i++)
  {
    Real rho, H;
    if (_rhouA_old[i] * _normals[i] > 0)
    {
      const Real H_junction = _H_junction_uo.getJunctionStagnationEnthalpy();
      const Real h = H_junction - 0.5 * _vel[i] * _vel[i];
      const Real p = _fp.p_from_h_s(h, _s_junction[0]);
      rho = _rhoA[i] / _A[i];
      const Real e = _fp.e_from_p_rho(p, rho);
      const Real E = e + 0.5 * _vel[i] * _vel[i];
      H = E + p / rho;
    }
    else
    {
      const Real H_junction = _H_junction_uo.getJunctionStagnationEnthalpy();
      H = H_junction;
      const Real p0_junction = _fp.p_from_h_s(H_junction, _s_junction[0]);
      const Real p0 = (1 - _K[i]) * p0_junction + _K[i] * _p[i];
      Real s, ds_dH, ds_dp0;
      _fp.s_from_h_p(H, p0, s, ds_dH, ds_dp0);
      const Real h = H - 0.5 * _vel[i] * _vel[i];
      const Real p = _fp.p_from_h_s(h, s);
      rho = _fp.rho_from_p_s(p, s);
    }

    re(i) = rho * H * _A[i] * _vel[i] * _normals[i];
  }
  re *= _var.scalingFactor();
  _assembly.cacheResidualNodes(re, dofs);
}

Real JunctionEnergyConstraint::computeQpResidual(Moose::ConstraintType /*type*/) { return 0; }

void
JunctionEnergyConstraint::computeJacobian(SparseMatrix<Number> & /*jacobian*/)
{
  MooseVariable & var_rhoA = _sys.getFieldVariable<Real>(_tid, _rhoA_var_number);
  MooseVariable & var_rhouA = _sys.getFieldVariable<Real>(_tid, _rhouA_var_number);
  MooseVariable & var_rhoEA = _sys.getFieldVariable<Real>(_tid, _rhoEA_var_number);
  MooseVariableScalar & var_s_junction = _sys.getScalarVariable(_tid, _s_junction_var_number);

  auto && dofs = _var.dofIndices();
  auto && dofs_rhoA = var_rhoA.dofIndices();
  auto && dofs_rhouA = var_rhouA.dofIndices();
  auto && dofs_rhoEA = var_rhoEA.dofIndices();
  auto && dofs_s_junction = var_s_junction.dofIndices();

  DenseMatrix<Number> J_rhoA(dofs.size(), dofs_rhoA.size());
  DenseMatrix<Number> J_rhouA(dofs.size(), dofs_rhouA.size());
  DenseMatrix<Number> J_rhoEA(dofs.size(), dofs_rhoEA.size());
  DenseMatrix<Number> J_s_junction(dofs.size(), dofs_s_junction.size());

  J_rhoA.zero();
  J_rhouA.zero();
  J_rhoEA.zero();
  J_s_junction.zero();

  const Real H_junction = _H_junction_uo.getJunctionStagnationEnthalpy();
  const std::vector<Real> & dH_junction_drhoA =
      _H_junction_uo.getJunctionStagnationEnthalpyMassDerivatives();
  const std::vector<Real> & dH_junction_drhouA =
      _H_junction_uo.getJunctionStagnationEnthalpyMomentumDerivatives();
  const std::vector<Real> & dH_junction_drhoEA =
      _H_junction_uo.getJunctionStagnationEnthalpyEnergyDerivatives();

  for (unsigned int i = 0; i < dofs.size(); i++)
  {
    if (_rhouA_old[i] * _normals[i] > 0)
    {
      const Real h = H_junction - 0.5 * _vel[i] * _vel[i];

      Real p, dp_dh, dp_ds_junction;
      _fp.p_from_h_s(h, _s_junction[0], p, dp_dh, dp_ds_junction);

      const Real An = _A[i] * _normals[i];

      const Real rho = _rhoA[i] / _A[i];

      Real e, de_dp, de_drho;
      _fp.e_from_p_rho(p, rho, e, de_dp, de_drho);
      const Real de_ds_junction = de_dp * dp_ds_junction;

      const Real E = e + 0.5 * _vel[i] * _vel[i];
      const Real dE_de = 1;
      const Real dE_dvel = _vel[i];
      const Real dE_ds_junction = dE_de * de_ds_junction;

      const Real H = E + p / rho;
      const Real dH_dE = 1;
      const Real dH_dp = 1.0 / rho;
      const Real dH_drho = -p / (rho * rho);
      const Real dH_ds_junction = dH_dE * dE_ds_junction + dH_dp * dp_ds_junction;

      J_s_junction(i, 0) = rho * dH_ds_junction * _vel[i] * An;

      for (unsigned int j = 0; j < dofs.size(); j++)
      {
        Real drho_drhoA, dvel_drhoA, dvel_drhouA;
        if (j == i)
        {
          drho_drhoA = 1.0 / _A[i];
          dvel_drhoA = THM::dvel_darhoA(_rhoA[i], _rhouA[i]);
          dvel_drhouA = THM::dvel_darhouA(_rhoA[i]);
        }
        else
        {
          drho_drhoA = 0;
          dvel_drhoA = 0;
          dvel_drhouA = 0;
        }

        const Real dh_drhoA = dH_junction_drhoA[j] - _vel[i] * dvel_drhoA;
        const Real dh_drhouA = dH_junction_drhouA[j] - _vel[i] * dvel_drhouA;
        const Real dh_drhoEA = dH_junction_drhoEA[j];

        const Real dp_drhoA = dp_dh * dh_drhoA;
        const Real dp_drhouA = dp_dh * dh_drhouA;
        const Real dp_drhoEA = dp_dh * dh_drhoEA;

        const Real de_drhoA = de_dp * dp_drhoA + de_drho * drho_drhoA;
        const Real de_drhouA = de_dp * dp_drhouA;
        const Real de_drhoEA = de_dp * dp_drhoEA;

        const Real dE_drhoA = dE_de * de_drhoA + dE_dvel * dvel_drhoA;
        const Real dE_drhouA = dE_de * de_drhouA + dE_dvel * dvel_drhouA;
        const Real dE_drhoEA = dE_de * de_drhoEA;

        const Real dH_drhoA = dH_dE * dE_drhoA + dH_dp * dp_drhoA + dH_drho * drho_drhoA;
        const Real dH_drhouA = dH_dE * dE_drhouA + dH_dp * dp_drhouA;
        const Real dH_drhoEA = dH_dE * dE_drhoEA + dH_dp * dp_drhoEA;

        J_rhoA(i, j) =
            (drho_drhoA * _vel[i] * H + rho * dvel_drhoA * H + rho * _vel[i] * dH_drhoA) * _A[i] *
            _normals[i];
        J_rhouA(i, j) = rho * (dvel_drhouA * H + _vel[i] * dH_drhouA) * _A[i] * _normals[i];
        J_rhoEA(i, j) = rho * _vel[i] * dH_drhoEA * _A[i] * _normals[i];
      }
    }
    else
    {
      const Real H = H_junction;

      Real p0_junction, dp0_junction_dH_junction, dp0_junction_ds_junction;
      _fp.p_from_h_s(H_junction,
                     _s_junction[0],
                     p0_junction,
                     dp0_junction_dH_junction,
                     dp0_junction_ds_junction);

      Real pi, dpi_dv, dpi_de;
      _fp.p_from_v_e(_v[i], _e[i], pi, dpi_dv, dpi_de);
      const Real dpi_drhoAi = dpi_dv * THM::dv_darhoA(_A[i], _rhoA[i]) +
                              dpi_de * THM::de_darhoA(_rhoA[i], _rhouA[i], _rhoEA[i]);
      const Real dpi_drhouAi = dpi_de * THM::de_darhouA(_rhoA[i], _rhouA[i]);
      const Real dpi_drhoEAi = dpi_de * THM::de_darhoEA(_rhoA[i]);

      const Real p0 = (1 - _K[i]) * p0_junction + _K[i] * _p[i];
      const Real dp0_dp0_junction = (1 - _K[i]);
      const Real dp0_dpi = _K[i];
      const Real dp0_ds_junction = dp0_dp0_junction * dp0_junction_ds_junction;

      Real s, ds_dH, ds_dp0;
      _fp.s_from_h_p(H, p0, s, ds_dH, ds_dp0);
      const Real ds_ds_junction = ds_dp0 * dp0_ds_junction;

      const Real dvel_drhoAi = -_rhouA[i] / (_rhoA[i] * _rhoA[i]);
      const Real dvel_drhouAi = 1.0 / _rhoA[i];

      const Real h = H - 0.5 * _vel[i] * _vel[i];
      const Real dh_dH = 1;
      const Real dh_dvel = -_vel[i];

      Real p, dp_dh, dp_ds;
      _fp.p_from_h_s(h, s, p, dp_dh, dp_ds);
      const Real dp_ds_junction = dp_ds * ds_ds_junction;

      Real rho, drho_dp, drho_ds;
      _fp.rho_from_p_s(p, s, rho, drho_dp, drho_ds);
      const Real drho_ds_junction = drho_dp * dp_ds_junction + drho_ds * ds_ds_junction;

      const Real An = _A[i] * _normals[i];

      J_s_junction(i, 0) = drho_ds_junction * H * _vel[i] * An;

      for (unsigned int j = 0; j < dofs.size(); j++)
      {
        Real dvel_drhoAj, dvel_drhouAj, dpi_drhoAj, dpi_drhouAj, dpi_drhoEAj;
        if (j == i)
        {
          dvel_drhoAj = dvel_drhoAi;
          dvel_drhouAj = dvel_drhouAi;
          dpi_drhoAj = dpi_drhoAi;
          dpi_drhouAj = dpi_drhouAi;
          dpi_drhoEAj = dpi_drhoEAi;
        }
        else
        {
          dvel_drhoAj = 0;
          dvel_drhouAj = 0;
          dpi_drhoAj = 0;
          dpi_drhouAj = 0;
          dpi_drhoEAj = 0;
        }

        const Real dH_drhoAj = dH_junction_drhoA[j];
        const Real dH_drhouAj = dH_junction_drhouA[j];
        const Real dH_drhoEAj = dH_junction_drhoEA[j];

        const Real dp0_junction_drhoAj = dp0_junction_dH_junction * dH_drhoAj;
        const Real dp0_junction_drhouAj = dp0_junction_dH_junction * dH_drhouAj;
        const Real dp0_junction_drhoEAj = dp0_junction_dH_junction * dH_drhoEAj;

        const Real dp0_drhoAj = dp0_dp0_junction * dp0_junction_drhoAj + dp0_dpi * dpi_drhoAj;
        const Real dp0_drhouAj = dp0_dp0_junction * dp0_junction_drhouAj + dp0_dpi * dpi_drhouAj;
        const Real dp0_drhoEAj = dp0_dp0_junction * dp0_junction_drhoEAj + dp0_dpi * dpi_drhoEAj;

        const Real ds_drhoAj = ds_dH * dH_drhoAj + ds_dp0 * dp0_drhoAj;
        const Real ds_drhouAj = ds_dH * dH_drhouAj + ds_dp0 * dp0_drhouAj;
        const Real ds_drhoEAj = ds_dH * dH_drhoEAj + ds_dp0 * dp0_drhoEAj;

        const Real dh_drhoAj = dh_dH * dH_drhoAj + dh_dvel * dvel_drhoAj;
        const Real dh_drhouAj = dh_dH * dH_drhouAj + dh_dvel * dvel_drhouAj;
        const Real dh_drhoEAj = dh_dH * dH_drhoEAj;

        const Real dp_drhoAj = dp_dh * dh_drhoAj + dp_ds * ds_drhoAj;
        const Real dp_drhouAj = dp_dh * dh_drhouAj + dp_ds * ds_drhouAj;
        const Real dp_drhoEAj = dp_dh * dh_drhoEAj + dp_ds * ds_drhoEAj;

        const Real drho_drhoAj = drho_dp * dp_drhoAj + drho_ds * ds_drhoAj;
        const Real drho_drhouAj = drho_dp * dp_drhouAj + drho_ds * ds_drhouAj;
        const Real drho_drhoEAj = drho_dp * dp_drhoEAj + drho_ds * ds_drhoEAj;

        J_rhoA(i, j) =
            (drho_drhoAj * _vel[i] * H + rho * dvel_drhoAj * H + rho * _vel[i] * dH_drhoAj) *
            _A[i] * _normals[i];
        J_rhouA(i, j) =
            (drho_drhouAj * _vel[i] * H + rho * dvel_drhouAj * H + rho * _vel[i] * dH_drhouAj) *
            _A[i] * _normals[i];
        J_rhoEA(i, j) =
            (drho_drhoEAj * _vel[i] * H + rho * _vel[i] * dH_drhoEAj) * _A[i] * _normals[i];
      }
    }
  }

  _assembly.cacheJacobianBlock(J_rhoA, dofs, dofs_rhoA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(J_rhouA, dofs, dofs_rhouA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(J_rhoEA, dofs, dofs_rhoEA, _var.scalingFactor());
  _assembly.cacheJacobianBlock(J_s_junction, dofs, dofs_s_junction, _var.scalingFactor());
}

Real JunctionEnergyConstraint::computeQpJacobian(Moose::ConstraintJacobianType /*type*/)
{
  return 0;
}

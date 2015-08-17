#include "Numerics.h"
#include "SBTL.h"

extern "C" double SIGMA_TS(double t);
extern "C" double U_V_TMAX_AUX_T(double vt);
extern "C" double U2_V_AUX_T(double vt);
extern "C" double V_U_PMAX_AUX(double u);
extern "C" double V1_U_AUX(double u);
extern "C" int    SAT_U1_SPL(double u, double& ps, double& ts, double& vl);
extern "C" int    SAT_V2_SPL_T(double vt, double& ps, double& ts, double& uv);
extern "C" int    SAT_VU_SPL(double v, double u, double& ps, double& ts, double& x, double& vl, double& vv, double& vvt, double& ul, double& uv);

Real
Reynolds(Real volume_fraction, Real rho, Real v, Real Dh, Real visc)
{
  return volume_fraction * rho * std::fabs(v) * Dh / visc;
}

Real
Prandtl(Real cp, Real mu, Real k)
{
  return cp * mu / k;
}

Real
Grashof(Real beta, Real dT, Real Dh, Real rho_l, Real visc_l)
{
  // Eq. 6-17
  return gravity_const * beta * dT * std::pow(Dh, 3) * (rho_l * rho_l) / (visc_l * visc_l);
}

Real
wallHeatTransferCoefficient(Real Nu, Real k, Real Dh)
{
  return Nu * k / Dh;
}

Real
surfaceTension(Real temperature)
{
  return SIGMA_TS(temperature) * 1e-3;               // [ N/m]
}

int
ireg_vu_SBTL95(double v, double u, double& vt, double& ps, double& ts, double& x, double& v1, double& v2, double& v2t, double& u1, double& u2)
{
    double umax,vmin,vmax,ps_,ts_,u2_;
    int ireg,ierr;
//
    static const double uc=2015.73452419;
    static const double vc=1./322.;
    static const double u2max=2603.22; //2603.21611007452;
    static const double upmaxtmax=3804.0140783331776;
    static const double vpmaxtmax=5.6899918862883302e-3;
    static const double upmintmax=4055.2632245319014;
    static const double vpmintmax=961.33913797178559;
    static const double vgmin=1.58121e-3;
    static const double vgmax=1189.01;
    //saturation states at 0�C (exactly)
    static const double u1tr=-4.1286719082445494e-2;
    static const double u2tr=2374.9064121738693;
    static const double v1tr=1.0002081892350456e-3;
    static const double v2tr=206.13388052154701;
    static const double qtr=1./(u2tr-u1tr);
//
    ireg=IREG_ERR;
    vt =ERR_VAL;
    ps =ERR_VAL;
    ts =ERR_VAL;
    x  =ERR_VAL;
    v1 =ERR_VAL;
    v2 =ERR_VAL;
    v2t=ERR_VAL;
    u1 =ERR_VAL;
    u2 =ERR_VAL;
//
    if(v>vc) {
        if(u>uc) {
            if(u>u2max) {
                if(u<upmaxtmax) {
                    if(v>vpmaxtmax) {
                        if(v<v2tr) {
                            vt=log(v);
                            ireg=IREG_G;
                        } else {
                            if(v<=vpmintmax) {
                                vt=log(v);
                                ireg=IREG_G;
                            }
                        }
                    } else {
                        if(v>=vgmin) {
                            vt=log(v);
                            ireg=IREG_G;
                        }
                    }
                } else if(u<=upmintmax) {
                    vt=log(v);
                    umax=U_V_TMAX_AUX_T(vt);
                    if(u<=umax) {
                        ireg=IREG_G;
                    }
                }
            } else {
                if(v<=v2tr) {
                    vt=log(v);
                    u2_=U2_V_AUX_T(vt);
                    if(u>(u2_+0.1)) {
                        ireg=IREG_G;
                    } else if(u<(u2_-0.1)) {
                        vmax=v1tr+(u-u1tr)*qtr*(v2tr-v1tr);
                        if(v<=vmax) {
                            ierr=SAT_VU_SPL(v,u,ps,ts,x,v1,v2,v2t,u1,u2);
                            if(ierr==I_OK) ireg=IREG_TP;
                        }
                    } else {
                        ierr=SAT_V2_SPL_T(vt, ps_, ts_, u2_);
                        if(u>=u2_) {
                            ireg=IREG_G;
                        } else {
                            vmax=v1tr+(u-u1tr)*qtr*(v2tr-v1tr);
                            if(v<=vmax) {
                                ierr=SAT_VU_SPL(v,u,ps,ts,x,v1,v2,v2t,u1,u2);
                                if(ierr==I_OK) ireg=IREG_TP;
                            }
                        }
                    }
                } else {
                    if(u>u2tr) {
                        if(v<=vgmax) {
                            vt=log(v);
                            ireg=IREG_G;
                        }
                    }
                }
            }
        } else {
            vmax=v1tr+(u-u1tr)*qtr*(v2tr-v1tr);
            if(v<=vmax) {
                ierr=SAT_VU_SPL(v,u,ps,ts,x,v1,v2,v2t,u1,u2);
                if(ierr==I_OK) ireg=IREG_TP;
            } else {
                ireg=IREG_ERR;
            }
        }
    } else {
        vmin=V_U_PMAX_AUX(u);
        if(v>=vmin*0.999) {
            if(u>=uc) {
                vt=log(v);
                ireg=IREG_G;
            } else {
                v1=V1_U_AUX(u);
                if(v>(v1+0.0005)) {
                    vmax=v1tr+(u-u1tr)*qtr*(v2tr-v1tr);
                    if(v<=vmax) {
                        ierr=SAT_VU_SPL(v,u,ps,ts,x,v1,v2,v2t,u1,u2);
                        if(ierr==I_OK) ireg=IREG_TP;
                    } else {
                        ireg=IREG_ERR;
                    }
                } else if(v<(v1-0.0005)) {
                    ireg=IREG_L;
                } else {
                    ierr=SAT_U1_SPL(u, ps_, ts_, v1);
                    if(v>v1) {
                        vmax=v1tr+(u-u1tr)*qtr*(v2tr-v1tr);
                        if(v<=vmax) {
                            ierr=SAT_VU_SPL(v,u,ps,ts,x,v1,v2,v2t,u1,u2);
                            if(ierr==I_OK) ireg=IREG_TP;
                        } else {
                            ireg=IREG_ERR;
                        }
                    } else {
                        ireg=IREG_L;
                    }
                }
            }
        } else {
            ireg=IREG_ERR;
        }
    }
    return ireg;
}

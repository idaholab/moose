#include "UsrFunc.h"
#include "Factory.h"

/**
 *   Manufactured solution for advection-diffusion-reaction problem-1.
 */
Number ManSol4ADR1(const Point& p, Real& A0, Real& B0, Real& C0, Real& omega, Real &t, bool& is_transient)
{
   Number f = A0*sin(3.*libMesh::pi*p(0))+B0*sin(3.*libMesh::pi*p(1))+C0*sin(libMesh::pi*p(0))*sin(libMesh::pi*p(1));
   if(is_transient) return f*sin(omega*libMesh::pi*t);
   else             return f;
}

Number ManSol4ADR1src(const Point& p, Real& A0, Real& B0, Real& C0,
                                      Real& Au, Real& Bu, Real& Cu,
                                      Real& Av, Real& Bv, Real& Cv,
                                      Real& Ak, Real& Bk, Real& Ck, Real& W, Real &t, bool& is_transient)
{
   Real x = p(0);
   Real y = p(1);

   if(is_transient)
      return libMesh::pi*(Ck*sin(libMesh::pi*t*W)*(3.*B0*cos(3.*libMesh::pi*y) + C0*cos(libMesh::pi*y)*sin(libMesh::pi*x))
           + (Av + Bv*x + Cv*y)*sin(libMesh::pi*t*W)*(3.*B0*cos(3.*libMesh::pi*y) + C0*cos(libMesh::pi*y)*sin(libMesh::pi*x))
           + Bk*sin(libMesh::pi*t*W)*(3.*A0*cos(3.*libMesh::pi*x) + C0*cos(libMesh::pi*x)*sin(libMesh::pi*y))
           + (Au + Bu*x + Cu*y)*sin(libMesh::pi*t*W)*(3.*A0*cos(3.*libMesh::pi*x) + C0*cos(libMesh::pi*x)*sin(libMesh::pi*y))
           - libMesh::pi*(Ak + Bk*x + Ck*y)*sin(libMesh::pi*t*W)*(9.*A0*sin(3.*libMesh::pi*x)
           + C0*sin(libMesh::pi*x)*sin(libMesh::pi*y)) + W*cos(libMesh::pi*t*W)*(A0*sin(3.*libMesh::pi*x)
           + C0*sin(libMesh::pi*x)*sin(libMesh::pi*y) + B0*sin(3.*libMesh::pi*y))
           - libMesh::pi*(Ak + Bk*x + Ck*y)*sin(libMesh::pi*t*W)*(C0*sin(libMesh::pi*x)*sin(libMesh::pi*y)
           + 9.*B0*sin(3.*libMesh::pi*y)));
   else
      return libMesh::pi*(3.*A0*(Au + Bk + Bu*x + Cu*y)*cos(3.*libMesh::pi*x)
           + 3.*B0*(Av + Ck + Bv*x + Cv*y)*cos(3.*libMesh::pi*y)
           - 9.*A0*libMesh::pi*(Ak + Bk*x + Ck*y)*sin(3.*libMesh::pi*x)
           + C0*(Au + Bk + Bu*x + Cu*y)*cos(libMesh::pi*x)*sin(libMesh::pi*y)
           + C0*sin(libMesh::pi*x)*((Av + Ck + Bv*x + Cv*y)*cos(libMesh::pi*y)
           - 2.*libMesh::pi*(Ak + Bk*x + Ck*y)*sin(libMesh::pi*y))
           - 9.*B0*libMesh::pi*(Ak + Bk*x + Ck*y)*sin(3.*libMesh::pi*y));
}

Number ManSol4ADR1exv(const Point& p,
  		      const InputParameters&,  // parameters, not needed
  		      const std::string&, // sys_name, not needed
  		      const std::string&) // unk_name, not needed
{
   Real A0=1.,B0=1.2,C0=0.8;
   return A0*sin(3.*libMesh::pi*p(0))+B0*sin(3.*libMesh::pi*p(1))+C0*sin(libMesh::pi*p(0))*sin(libMesh::pi*p(1));
}

Gradient ManSol4ADR1exd(const Point& p,
  		        const InputParameters&,  // parameters, not needed
  		        const std::string&, // sys_name, not needed
  		        const std::string&) // unk_name, not needed
{
   Real A0=1.,B0=1.2,C0=0.8;
   Gradient gradu;

   gradu(0) = 3.*libMesh::pi*A0*cos(3.*libMesh::pi*p(0))+C0*libMesh::pi*sin(libMesh::pi*p(1))*cos(libMesh::pi*p(0));
   gradu(1) = 3.*libMesh::pi*B0*cos(3.*libMesh::pi*p(1))+C0*libMesh::pi*cos(libMesh::pi*p(1))*sin(libMesh::pi*p(0));
   gradu(2) = 0.;

   return gradu;
}

/**
 *   Manufactured solution for advection-diffusion-reaction problem-2.
 */
Number ManSol4ADR2(const Point& p, Real& A0, Real& B0, Real& C0, Real& w, Real &t)
{
   Real x = p(0);
   Real y = p(1);
   Real r = sqrt(pow(x-A0,2)+pow(y-B0,2));
   if(r>C0)
      return 0.0;
   else
      return ((1.-cos(2.*libMesh::pi*(C0-sqrt(pow(-A0+x,2)+pow(-B0+y,2)))))*(1.+sin(libMesh::pi*(-0.5+t*w))))/4.;
}

Number ManSol4ADR2src(const Point& p, Real& A0, Real& B0, Real& C0,
                                      Real& Au, Real& Bu, Real& Cu,
                                      Real& Av, Real& Bv, Real& Cv,
                                      Real& Ak, Real& Bk, Real& Ck, Real& w, Real &t)
{
   Real x = p(0);
   Real y = p(1);
   Real r = sqrt(pow(x-A0,2)+pow(y-B0,2));
   if(r>C0)
      return 0.0;
   else
      return (libMesh::pi*((4.*pow(sin((libMesh::pi*t*w)/2.),2)*(2.*libMesh::pi*sqrt(pow(A0-x,2)
            +pow(B0-y,2))*(Ak+Bk*x+Ck*y)*cos(2.*libMesh::pi*(C0-sqrt(pow(A0-x,2)+pow(B0-y,2))))
            +(-Ak+Av*B0+B0*Ck-(Au+2.*Bk)*x+B0*Bv*x-Bu*pow(x,2)-(Av+2.*Ck-B0*Cv+(Bv+Cu)*x)*y-Cv*pow(y,2)
            +A0*(Au+Bk+Bu*x+Cu*y))*sin(2.*libMesh::pi*(C0-sqrt(pow(A0-x,2)+pow(B0-y,2))))))/sqrt(pow(A0-x,2)
            +pow(B0-y,2))+(w*(2.*sin(libMesh::pi*t*w)-sin(libMesh::pi*(2.*C0+t*w-2.*sqrt(pow(A0-x,2)
            +pow(B0-y,2))))-sin(libMesh::pi*(-2.*C0+t*w+2.*sqrt(pow(A0-x,2)+pow(B0-y,2))))))/2.))/4.;
}

Number ManSol4ADR2exv(const Point& p,
  		      const InputParameters &parameters,  // parameters, not needed
  		      const std::string&, // sys_name, not needed
  		      const std::string&) // unk_name, not needed
{
   Real A0=0.5,B0=0.5,C0=0.25,w=2.;
   Real time = parameters.get<Real>("time");
   return ManSol4ADR2(p,A0,B0,C0,w,time);
}

Gradient ManSol4ADR2exd(const Point& p,
  		        const InputParameters &parameters,  // parameters, not needed
  		        const std::string&, // sys_name, not needed
  		        const std::string&) // unk_name, not needed
{
   Real A0=0.5,B0=0.5,C0=0.25,w=2.;
   Real t = parameters.get<Real>("time");

   Gradient gradu;

   gradu(0) = 0.;
   gradu(1) = 0.;
   gradu(2) = 0.;

   Real x = p(0);
   Real y = p(1);
   Real r = sqrt(pow(x-A0,2)+pow(y-B0,2));
   if(r<=C0)
   {
      gradu(0) = (libMesh::pi*(A0-x)*pow(sin((libMesh::pi*t*w)/2.),2)*sin(2*libMesh::pi*(C0-sqrt(pow(A0-x,2)+pow(B0-y,2)))))/sqrt(pow(A0-x,2)+pow(B0-y,2));
      gradu(1) = (libMesh::pi*(B0-y)*pow(sin((libMesh::pi*t*w)/2.),2)*sin(2*libMesh::pi*(C0-sqrt(pow(A0-x,2)+pow(B0-y,2)))))/sqrt(pow(A0-x,2)+pow(B0-y,2));
   }

   return gradu;
}

Number ManSolzeroV(const Point&,  // p not needed
  		   const InputParameters&,  // parameters, not needed
  		   const std::string&, // sys_name, not needed
  		   const std::string&) // unk_name, not needed
{
   return 0.0;
}

Gradient ManSolzeroG(const Point&,  // p not needed
  		     const InputParameters&,  // parameters, not needed
  		     const std::string&, // sys_name, not needed
  		     const std::string&) // unk_name, not needed
{
   Gradient gradu;

   gradu(0) = 0.;
   gradu(1) = 0.;
   gradu(2) = 0.;

   return gradu;
}

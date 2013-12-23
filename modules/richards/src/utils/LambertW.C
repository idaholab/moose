/*
  Implementation of Lambert W function

  Copyright (C) 2009 Darko Veberic, darko.veberic@ung.si

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  Changes:
  30 Nov 2012: bug fix for W0(0), reported by Timothy R. Waters
*/

#include <iostream>
#include <cmath>
#include "LambertW.h"

using namespace std;

#if 0
#  define NAMESPACE_UTL_BEGIN namespace utl {
#  define NAMESPACE_UTL_END }
#else
#  define NAMESPACE_UTL_BEGIN
#  define NAMESPACE_UTL_END
#endif


NAMESPACE_UTL_BEGIN

  namespace LambertWDetail {


    template<int n>
    inline double BranchPointPolynomial(const double p);


    /*template<>
    inline
    double
    BranchPointPolynomial<0>(const double / *p* /)
    {
      return -1;                     // 0
    }


    template<>
    inline
    double
    BranchPointPolynomial<1>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1                        // 1
        );
    }


    template<>
    inline
    double
    BranchPointPolynomial<2>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0   // 2
        ));
    }


    template<>
    inline
    double
    BranchPointPolynomial<3>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0   // 3
        )));
    }


    template<>
    inline
    double
    BranchPointPolynomial<4>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1   // 4
        ))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<5>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1   // 5
        )))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<6>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1   // 6
        ))))));
    }*/


    template<>
    inline
    double
    BranchPointPolynomial<7>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1   // 7
        )))))));
    }


    /*template<>
    inline
    double
    BranchPointPolynomial<8>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2   // 8
        ))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<9>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2   // 9
        )))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<10>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2   // 10
        ))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<11>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2   // 11
        )))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<12>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2   // 12
        ))))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<13>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2   // 13
        )))))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<14>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3   // 14
        ))))))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<15>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3 + // 14
        p*( 0.44247306181462090e-3   // 15
        )))))))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<16>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3 + // 14
        p*( 0.44247306181462090e-3 + // 15
        p*(-0.29267722472962746e-3   // 16
        ))))))))))))))));
    }


    template<>
    inline
    double
    BranchPointPolynomial<17>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3 + // 14
        p*( 0.44247306181462090e-3 + // 15
        p*(-0.29267722472962746e-3 + // 16
        p*( 0.19438727605453930e-3   // 17
        ))))))))))))))))); 
    }


    template<>
    inline
    double
    BranchPointPolynomial<18>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3 + // 14
        p*( 0.44247306181462090e-3 + // 15
        p*(-0.29267722472962746e-3 + // 16
        p*( 0.19438727605453930e-3 + // 17
        p*(-0.12957426685274883e-3   // 18
        )))))))))))))))))); 
    }


    template<>
    inline
    double
    BranchPointPolynomial<19>(const double p)
    {
      return
           -1 +                      // 0
        p*( 1 +                      // 1
        p*(-0.333333333333333333e0 + // 2
        p*( 0.152777777777777777e0 + // 3
        p*(-0.79629629629629630e-1 + // 4
        p*( 0.44502314814814814e-1 + // 5
        p*(-0.25984714873603760e-1 + // 6
        p*( 0.15635632532333920e-1 + // 7
        p*(-0.96168920242994320e-2 + // 8
        p*( 0.60145432529561180e-2 + // 9
        p*(-0.38112980348919993e-2 + // 10
        p*( 0.24408779911439826e-2 + // 11
        p*(-0.15769303446867841e-2 + // 12
        p*( 0.10262633205076071e-2 + // 13
        p*(-0.67206163115613620e-3 + // 14
        p*( 0.44247306181462090e-3 + // 15
        p*(-0.29267722472962746e-3 + // 16
        p*( 0.19438727605453930e-3 + // 17
        p*(-0.12957426685274883e-3 + // 18
        p*( 0.86650358052081260e-4   // 19
        )))))))))))))))))));
    }*/

    /*
      -1
       1
      -0.333333333333333333e0
       0.152777777777777777e0
      -0.79629629629629630e-1
       0.44502314814814814e-1
      -0.25984714873603760e-1
       0.15635632532333920e-1
      -0.96168920242994320e-2
       0.60145432529561180e-2
      -0.38112980348919993e-2
       0.24408779911439826e-2
      -0.15769303446867841e-2
       0.10262633205076071e-2
      -0.67206163115613620e-3
       0.44247306181462090e-3
      -0.29267722472962746e-3
       0.19438727605453930e-3
      -0.12957426685274883e-3
       0.86650358052081260e-4
    */

    template<int order>
    inline double AsymptoticExpansion(const double a, const double b);


    /*template<>
    inline
    double
    AsymptoticExpansion<0>(const double a, const double b)
    {
      return a - b;
    }


    template<>
    inline
    double
    AsymptoticExpansion<1>(const double a, const double b)
    {
      return a - b + b / a;
    }


    template<>
    inline
    double
    AsymptoticExpansion<2>(const double a, const double b)
    {
      const double ia = 1 / a;
      return a - b + b / a * (1 + ia * 0.5*(-2 + b));
    }


    template<>
    inline
    double
    AsymptoticExpansion<3>(const double a, const double b)
    { 
      const double ia = 1 / a;
      return a - b + b / a *
        (1 + ia *
          (0.5*(-2 + b) + ia *
             1/6.*(6 + b*(-9 + b*2))
          )
        );
    }


    template<>
    inline
    double
    AsymptoticExpansion<4>(const double a, const double b)
    { 
      const double ia = 1 / a;
      return a - b + b / a *
        (1 + ia *
          (0.5*(-2 + b) + ia *
            (1/6.*(6 + b*(-9 + b*2)) + ia *
              1/12.*(-12 + b*(36 + b*(-22 + b*3)))
            )
          )
        );
    }*/


    template<>
    inline
    double
    AsymptoticExpansion<5>(const double a, const double b)
    {
      const double ia = 1 / a;
      return a - b + b / a *
        (1 + ia *
          (0.5*(-2 + b) + ia *
            (1/6.*(6 + b*(-9 + b*2)) + ia *
              (1/12.*(-12 + b*(36 + b*(-22 + b*3))) + ia *
                1/60.*(60 + b*(-300 + b*(350 + b*(-125 + b*12))))
              )
            )
          )
        );
    }


    template<int branch>
    class Branch {

    public:
      enum { eBranch = branch };

      template<int order>
      static double BranchPointExpansion(const double x)
      { return BranchPointPolynomial<order>(eSign * sqrt(2*(M_E*x+1))); }

      // Asymptotic expansion
      // Corless et al. 1996, de Bruijn (1981)
      template<int order>
      static
      double
      AsymptoticExpansion(const double x)
      {
        const double logsx = log(eSign * x);
        const double logslogsx = log(eSign * logsx);
        return LambertWDetail::AsymptoticExpansion<order>(logsx, logslogsx);
      }

      template<int n>
      static inline double RationalApproximation(const double x);

      // Logarithmic recursion
      template<int order>
      static inline double LogRecursion(const double x)
      { return LogRecursionStep<order>(log(eSign * x)); }

      // generic approximation valid to at least 5 decimal places
      static inline double Approximation(const double x);

    private:
      enum { eSign = 2*branch + 1 };

      template<int n>
      static inline double LogRecursionStep(const double logsx)
      { return logsx - log(eSign * LogRecursionStep<n-1>(logsx)); }

    };


    // Rational approximations

    /*template<>
    template<>
    inline
    double
    Branch<0>::RationalApproximation<0>(const double x)
    {
      return x*(60 + x*(114 + 17*x)) / (60 + x*(174 + 101*x));
    }*/


    template<>
    template<>
    inline
    double
    Branch<0>::RationalApproximation<1>(const double x)
    {
      // branch 0, valid for [-0.31,0.3]
      return
        x*(1 +
        x*(5.931375839364438 +
        x*(11.392205505329132 +
        x*(7.338883399111118 +
        x*0.6534490169919599)))) /
        (1 +
         x*(6.931373689597704 +
         x*(16.82349461388016 +
         x*(16.43072324143226 +
         x*5.115235195211697))));
    }


    /*template<>
    template<>
    inline
    double
    Branch<0>::RationalApproximation<2>(const double x)
    {
      // branch 0, valid for [-0.31,0.5]
      return
        x*(1 +
        x*(4.790423028527326 +
        x*(6.695945075293267 +
        x*2.4243096805908033))) /
        (1 +
         x*(5.790432723810737 +
         x*(10.986445930034288 +
         x*(7.391303898769326 +
         x*1.1414723648617864))));
    }*/


    template<>
    template<>
    inline
    double
    Branch<0>::RationalApproximation<3>(const double x)
    {
      // branch 0, valid for [0.3,7]
      return
        x*(1 +
        x*(2.4450530707265568 +
        x*(1.3436642259582265 +
        x*(0.14844005539759195 +
        x*0.0008047501729129999)))) /
        (1 +
         x*(3.4447089864860025 +
         x*(3.2924898573719523 +
         x*(0.9164600188031222 +
         x*0.05306864044833221))));
    }


    template<>
    template<>
    inline
    double
    Branch<-1>::RationalApproximation<4>(const double x)
    {
      // branch -1, valid for [-0.3,-0.05]
      return
        (-7.814176723907436 +
         x*(253.88810188892484 +
         x*657.9493176902304)) /
        (1 +
         x*(-60.43958713690808 +
         x*(99.98567083107612 +
         x*(682.6073999909428 +
         x*(962.1784396969866 +
         x*1477.9341280760887)))));
    }


    // stopping conditions

    template<>
    template<>
    inline
    double
    Branch<0>::LogRecursionStep<0>(const double logsx)
    {
      return logsx;
    }


    template<>
    template<>
    inline
    double
    Branch<-1>::LogRecursionStep<0>(const double logsx)
    {
      return logsx;
    }


    /** Approximate Lambert W function
      only \f$W_0(x)\f$ and \f$W_{-1}(x)\f$ have real values

      \param branch: valid values are 0 and -1
    */

    template<>
    inline
    double
    Branch<0>::Approximation(const double x)
    {
      return (x < 0.14546954290661823) ?
        ((x < -0.32358170806015724) ?
          BranchPointExpansion<7>(x) : RationalApproximation<1>(x))
        :
        ((x < 8.706658967856612) ?
          RationalApproximation<3>(x) : AsymptoticExpansion<5>(x));
    }


    template<>
    inline
    double
    Branch<-1>::Approximation(const double x)
    {
      return (x > -0.035) ?
        LogRecursion<5>(x) :
        ((x > -0.311) ?
          RationalApproximation<4>(x) :
          BranchPointExpansion<7>(x));
    }


    // iterations

    /*inline
    double
    HalleyStep(const double x, const double w)
    {
      const double ew = exp(w);
      const double wew = w * ew;
      const double wewx = wew - x;
      const double w1 = w + 1;
      return w - wewx / (ew * w1 - (w + 2) * wewx/(2*w1));
    }*/


    inline
    double
    FritschStep(const double x, const double w)
    {
      const double z = log(x/w) - w;
      const double w1 = w + 1;
      const double q = 2 * w1 * (w1 + 0.666666666666666666*z);
      const double eps = z / w1 * (q - z) / (q - 2*z);
      return w * (1 + eps);
    }


    /*template<
      double IterationStep(const double x, const double w)
    >
    inline
    double
    Iterate(const double x, double w, const double eps = 1e-6)
    {
      for (int i = 0; i < 100; ++i) {
        const double ww = IterationStep(x, w);
        if (fabs(ww - w) <= eps)
          return ww;
        w = ww;
      }
      cerr << "convergence not reached." << endl;
      return w;
    }*/


    template<
      double IterationStep(const double x, const double w)
    >
    struct Iterator {

      /*static
      double
      Do(const int n, const double x, double w)
      {
        for (int i = 0; i < n; ++i)
          w = IterationStep(x, w);
        return w;
      }

      template<int n>
      static
      double
      Do(const double x, double w)
      {
        for (int i = 0; i < n; ++i)
          w = IterationStep(x, w);
        return w;
      }*/

      template<int n, class = void>
      struct Depth {
        static double Recurse(const double x, const double w)
        { return Depth<n-1>::Recurse(x, IterationStep(x, w)); }
      };

      // stop condition
      template<class T>
      struct Depth<1, T> {
        static double Recurse(const double x, const double w)
        { return IterationStep(x, w); }
      };

      // identity
      template<class T>
      struct Depth<0, T> {
        static double Recurse(const double x, const double w)
        { return w; }
      };

    };


  } // LambertWDetail


  template<int branch>
  double
  LambertWApproximation(const double x)
  {
    return LambertWDetail::Branch<branch>::Approximation(x);
  }


  // instantiations
  template double LambertWApproximation<0>(const double x);
  template double LambertWApproximation<-1>(const double x);


  template<int branch>
  double
  LambertW(const double x)
  {
    if (branch == 0 && x == 0)
      return 0;
    return
      LambertWDetail::
        Iterator<LambertWDetail::FritschStep>::
          Depth<1>::
            Recurse(x, LambertWApproximation<branch>(x));
  }


  // instantiations
  template double LambertW<0>(const double x);
  template double LambertW<-1>(const double x);

NAMESPACE_UTL_END

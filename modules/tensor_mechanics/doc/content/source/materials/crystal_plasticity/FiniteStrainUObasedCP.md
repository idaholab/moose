# UserObject based Crystal Plasticity System

!syntax description /Materials/FiniteStrainUObasedCP

## Description

The UserObject based crystal plasticity system is designed to facilitate the implementation of different constitutive laws in a modular way. Both phenomenological constitutive models and dislocation-based constitutive models can be implemented through this system. This system consists of one material class `FiniteStrainUObasedCP` and four UserObject classes, namely `CrystalPlasticitySlipRate`, `CrystalPlasticitySlipResistance`, `CrystalPlasticityStateVarRateComponent` and `CrystalPlasticityStateVariable`.

The material class is based on plastic flow on individual slip systems to obtain the inelastic deformation in materials. The present formulation considers large deformation and is based on a stress update algorithm. Some of the important functions associated with this class are outlined below.

+Function `computeQpStress`+:

Input:

- 2$^{nd}$ Piola-Kirchhoff (PK2) stress in the intermediate configuration at previous increment (variable `_pk2_old`, symbol $T_n$);
- Plastic deformation gradient at previous increment (variable `_fp_old`, symbol $F^p_n$);
- State variables at previous increment;
- Current deformation gradient (variable `_dfgrd`, symbol $F$).

Output:

- Current PK2 stress (variable `_pk2`, symbol $T$);
- Current plastic deformation gradient (variable `_fp`, symbol $F^p$);
- Current slip system resistances.
- State variables are solved iteratively using a predictor corrector algorithm. $T$ is solved using Newton Raphson.

+Flowchart+:

i: At iteration i=0, $y^\alpha_i=y^\alpha_n$

ii: Calculate the residual r and Jacobian J=$\partial r/\partial T$ from the function `calc_resid_jacob`

iii: Update T as: $T_{i+1}=T_{i}-J^{-1}r$

iv: Check $||r||_2 <$ `_rtol` (Optional user defined parameter)

If False then go to Step ii

Else

v: Calculate $y^\alpha_{i+1}$ from function `updateSlipSystemResistanceAndStateVariable`

vi: Check $max|s^\alpha_{i+1}-s^\alpha_{i}|/|s^\alpha_{i}| <$ `_stol`  (Optional user defined parameter)

If False then go to Step ii

Else

vii: Obtain rotation tensor R

viii: Update rotation as $\tilde{R}=RQ$ where Q is the rotation tensor from Euler angles

Exit Function

+Function `calc_resid_jacob`+:
User can override this function.

Input:

- PK2 stress at previous iteration (variable `_pk2`, symbol $T_i$, i - iteration);
- Inverse of plastic deformation gradient at previous increment (variable `_fp_old_inv`, symbol $F^{p-1}_n$);
- Current deformation gradient (variable `_dfgrd`, symbol $F$).

Output:

- Residual (variable `resid`, symbol r);
- Jacobian (variable `jac`, symbol J);
- Updated inverse of plastic deformation gradient (variable `_fp_inv`, symbol $F^{p-1}$)

+Residual r as implemented+:

i. Get slip rates from userobject.

ii. Resultant slip increment (variable `eqv_slip_incr`) $\Delta \gamma_{res}=I-\sum\limits_m\sum
\limits_\alpha \left( \Delta \gamma^\alpha_m S_m^\alpha\right)$ where $S_m^\alpha$ is the flow direction.

iii. Current plastic component of deformation gradient $F^{p-1}=F^{p-1}_n\Delta \gamma_{res}$.

iv. Elastic component of deformation gradient in iteration i+1, $F^e=FF^{p-1}$.

vi: PK2 stress due to $T_i$ and associated slip increment in current iteration i+1 (variable `pk2_new`), $T^\prime=C:\frac{1}{2}\left(F^{eT}F^e-I\right)$.

vii. Residual $r=T_i-T^\prime$.

+Jacobian J formulation+:

i. $r=T_i-T^\prime$ and $J=\frac{\partial r}{\partial T_i}=I-\frac{\partial T^\prime}{\partial T_i}$.

ii. $T^\prime=C:E^e$ which gives $\frac{\partial T^\prime}{\partial T_i}=C: \frac{\partial E^e}{\partial T_i}$.

iii. $E^e=\frac{1}{2}\left(F^{eT}F^e -I\right)$ which gives in indicial notation $\frac{\partial E^e_{ij}}{\partial F^e_{kl}}=\frac{1}{2}\left( \delta_{il}F^e_{kj}+\delta_{jl}F^e_{ki}\right)$ where $\delta_{ij}$ is the Kroneckar delta.

iv. $F^e=FF^{p-1}$ which gives in indicial notation $\frac{\partial F^e_{ij}}{\partial F^{p-1}_{kl}}=F_{ik}\delta_{lj}$.

v. $F^{p-1}=F^{p-1}_n\Delta \gamma_{res}$ which gives $\frac{\partial F^{p-1}}{\partial \Delta \gamma_{res}}=F^{p-1}_n$.

vi. $\Delta \gamma_{res}=I-\sum\limits_m\sum
\limits_\alpha \left( \Delta \gamma^\alpha_m S_m^\alpha\right)$ which gives $\frac{\partial \Delta \gamma_{res}}{\partial \Delta \gamma^\alpha_m}=-\sum\limits_\alpha S_m^\alpha$.

vii. $\Delta \gamma^\alpha_m=f(\tau_m^\alpha)$  which gives $\frac{\partial \Delta \gamma^\alpha_m}{\partial \tau^\alpha_m}$.

viii. $\tau^\alpha=T_i:S_m$ gives $\frac{\partial \tau^\alpha_m}{\partial  T_i}=S_m^\alpha$.

ix. Hence $\frac{\partial T^\prime}{\partial T_i}=C: \frac{\partial E^e}{\partial F^e} \frac{\partial F^e}{\partial F^{p-1}} \frac{\partial F^{p-1}}{\partial \Delta \gamma_{res}}\sum\limits_m(\frac{\partial \Delta \gamma_{res}}{\partial \Delta \gamma^\alpha_m}\frac{\partial \Delta \gamma^\alpha_m}{\partial \tau^\alpha_m})\frac{\partial \tau^\alpha}{\partial  T_i}$.

+Jacobian J as implemented+:

i. Variable `dtaudpk2`, $\frac{\partial \tau^\alpha}{\partial  T_i}$.

ii. Variable `dfpinvdslip`, $\frac{\partial F^{p-1}}{\partial \Delta \gamma^\alpha}$.

iii. Variable `dfedfpinv`, $\frac{\partial F^e}{\partial F^{p-1}}$.

iv: Variable `deedfe`, $\frac{\partial E^e}{\partial F^e}$.

v. Variable `dfpinvdpk2`, $\frac{\partial F^{p-1}}{\partial T_i}=\frac{\partial F^{p-1}}{\partial \Delta \gamma^\alpha} \frac{\partial \Delta \gamma^\alpha}{\partial \tau^\alpha} \frac{\partial \tau^\alpha}{\partial  T_i}$.

vi. $\frac{\partial \Delta \gamma^\alpha_m}{\partial \tau^\alpha_m}$ obtained from slip rate userobject.

vii. Variable `jac`, $J=C:\frac{\partial E^e}{\partial F^e} \frac{\partial F^e}{\partial F^{p-1}} \frac{\partial F^{p-1}}{\partial T_i}$, followed by $J=\mathfrak{I}-J$ where $\mathfrak{I}$ is the fourth order identity tensor.

Update Cauchy stress (variable sig) $\sigma=F^e T_i F^{eT}/J^e$.

+Function `computeQpElasticityTensor`+:
Defines tangent moduli K and can be used as preconditioner for JFNK. User can override this function.

Input:

- Current plastic deformation gradient (variable `_fp`, symbol $F^p$);
- Current deformation gradient (variable `_dfgrd`, symbol $F$)

Output:

- Tangent moduli (variable `_Jacobian_mult`,symbol K).

+Formulation+:

i. $K=\frac{\partial \sigma}{\partial F}$.

ii. $\sigma=F^eTF^{eT}/J^e$ which gives $\frac{\partial \sigma}{\partial F}=\frac{1}{J^e}\left(\frac{\partial F^e}{\partial F}TF^{eT}+F^e\frac{\partial T}{\partial F}F^{eT}+F^eT\frac{\partial F^{eT}}{\partial F}\right)$.

iii. In indicial notation $\frac{\partial F^e_{ij}}{\partial F_{kl}}=\delta_{ki}F^{p-1}_{lj}$.

iv. $T=C:E^e$ which gives $\frac{\partial T}{\partial F}=\frac{\partial T}{\partial E^e}\frac{\partial E^e}{\partial F^e}\frac{\partial F^e}{\partial F}=C:\frac{\partial E^e}{\partial F^e}\frac{\partial F^e}{\partial F}$.

v. In indicial notation $\frac{\partial E^e_{ij}}{\partial F^e_{kl}}=\frac{1}{2}\left(\delta_{il}F^e_{kj}+\delta_{jl}F^e_{ki}\right)$.

+Implementation+:

i. Variable `dfedf` calculates $\frac{\partial F^e}{\partial F}$.

ii. Variable `deedfe` calculates $\frac{\partial E^e}{\partial F^e}$.

iii. Variable `dsigspk2dfe` calculates $F^{eT} C:\frac{\partial E^e}{\partial F^e} F^e$.

!syntax parameters /Materials/FiniteStrainUObasedCP

!syntax inputs /Materials/FiniteStrainUObasedCP

!syntax children /Materials/FiniteStrainUObasedCP

!bibtex bibliography

# NonlinearElasticSpring

!syntax description /Materials/NonlinearElasticSpring

## Description

Nonlinear elastic spring law: the force along each local axis is a function of the deformation along that axis, the same curve on loading and unloading, and the tangent is the slope of the function, see [SpringElements](/SpringElements.md).

### Choosing the parameters

- `backbone_functions` names three functions in the order of the local axes. The deformation is the x coordinate of the function: a [PiecewiseLinear](/PiecewiseLinear.md) function takes `axis = x` with the deformations in `x` and the forces in `y`, a [ParsedFunction](/MooseParsedFunction.md) uses `x`. The forces have the dimension force (force per area when `tributary_area` is given; moment in the rotational mode) and the deformations the dimension length (radian). The function is evaluated outside its data by the rule of the function, which for a PiecewiseLinear function is the nearest value unless `extrap = true`.
- One-sided springs are recipes on the function: zero values on the negative side give a tension-only spring, on the positive side a compression-only spring, and a zero stretch between two deformations a gap. On its open side the law gives its degree of freedom no stiffness; a load on that degree of freedom needs a parallel element with a stiffness or a prescribed value, else the system is singular.
- `mode`, `tributary_area` and `base_name` as for [LinearSpring](/LinearSpring.md).

!listing modules/solid_mechanics/test/tests/spring/static/nonlinear_elastic.i block=Functions Materials

!media media/solid_mechanics/spring_law_nonlinear_elastic.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_nonlinear_elastic_law
      caption=The nonlinear elastic law with the compression-only recipe dashed.

!syntax parameters /Materials/NonlinearElasticSpring

!syntax inputs /Materials/NonlinearElasticSpring

!syntax children /Materials/NonlinearElasticSpring

!bibtex bibliography

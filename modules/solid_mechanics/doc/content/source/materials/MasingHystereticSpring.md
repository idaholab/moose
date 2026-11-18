# MasingHystereticSpring

!syntax description /Materials/MasingHystereticSpring

## Description

Hysteretic spring law: the first loading along each local axis follows its backbone function, and every unloading and reloading branch follows the Masing rule of [!cite](masing1926eigenspannungen), the backbone scaled by two about the last reversal point, until the branch meets the backbone again, see [SpringElements](/SpringElements.md). The law keeps the last reversal only: a reversal inside a loop starts a new branch that returns to the backbone where it meets it, not to the branch it left (the extended Masing rules are not implemented); the inner-loop case of the [verification page](/spring_vandv.md) pins that. The law keeps, per axis, the deformation and the force at the last reversal, the loading direction and the side of the backbone the branch runs on.

### Choosing the parameters

- `backbone_functions` as for [NonlinearElasticSpring](/NonlinearElasticSpring.md), with the backbone odd in the deformation (the Masing rule assumes it): a bilinear backbone gives the loops of linear kinematic hardening, a multilinear one the loops of a Ramberg-Osgood-like curve.
- `mode`, `tributary_area` and `base_name` as for [LinearSpring](/LinearSpring.md).

!listing modules/solid_mechanics/test/tests/spring/static/hysteretic.i block=Functions Materials

!media media/solid_mechanics/spring_law_masing_hysteretic.png
      style=width:60%;margin-left:auto;margin-right:auto;
      id=fig_masing_law
      caption=The Masing rule on a bilinear backbone.

- Under a load, use `line_search = bt` in the executioner: the tangent changes from the backbone slope to the branch slope at a reversal, and the Newton iterations of that step can otherwise jump between the two. Under a prescribed deformation no line search is needed.

!syntax parameters /Materials/MasingHystereticSpring

!syntax inputs /Materials/MasingHystereticSpring

!syntax children /Materials/MasingHystereticSpring

!bibtex bibliography

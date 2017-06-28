# MatReaction
!syntax description /Kernels/MatReaction

Implements

$$
-L(v,a,b,\dots)\cdot v,
$$

where $L$ (`mob_name`) is a reaction rate, $v$ is either a coupled variable (`v`)
or - if not explicitly specified - the non-linear variable the kernel is operating on.

!syntax parameters /Kernels/MatReaction

!syntax inputs /Kernels/MatReaction

!syntax children /Kernels/MatReaction

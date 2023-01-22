# Algorithm Extension

The algorithm extension provides a means to display psuedo-code similar to the [latex algorithmicx package](https://ctan.math.illinois.edu/macros/latex/contrib/algorithmicx/algorithmicx.pdf). Algorithms are similar to [tables](extensions/table.md) in that users can assign captions, ids, and provide [shortcuts](extensions/autolink.md) to the algorithm.

!devel settings caption=Configuration options for the algorithm extension.
                module=MooseDocs.extensions.algorithm
                object=AlgorithmExtension

## The Algorithm Float

!devel! example caption=Example of assigning a caption and id
!algorithm caption=Test id=algotest
[!function!begin name=testFunction]
[!function!end]

Look at [!ref](algotest)
!devel-end!

!devel settings caption=Configuration options for the algorithm command.
                module=MooseDocs.extensions.algorithm
                object=AlgorithmCommand


## Line Commands

The algorithm command by itself is not very informative. Users should use the following line commands following the algorithm float definition. Below is an example using some of the commands described in the following sections. $\quad$

!devel! example
!algorithm caption=The Bellman-Kalaba algorithm id=bk
[!function!begin name=BellmanKalab param=$G$, $u$, $l$, $p$]
[!for!begin condition=$v\in V(G)$]
[!state text=$l(v) \leftarrow \infty$]
[!for!end]
[!state text=$l(u) \leftarrow 0$]
[!while!begin condition=$changed$ comment=Intial is $changed \leftarrow \text{True}$]
[!for!begin condition=$i \leftarrow 1, n$]
[!state text=$min \leftarrow l(v_i)$]
[!for!begin condition=$j \leftarrow 1, n$]
[!ifthen!if condition=$min > e(v_i, v_j) + l(v_j)$]
[!state text=$min \leftarrow e(v_i, v_j) + l(v_j)$]
[!state text=$p(i) \leftarrow v_j$]
[!ifthen!end]
[!for!end]
[!state text=$l’(i) \leftarrow min$]
[!for!end]
[!state text=$changed \leftarrow l \not= l’$]
[!state text=$l \leftarrow l’$]
[!while!end]
[!function!end]
!devel-end!


### Function Commands

!devel! example caption=Example of using function line commands
!algorithm
[!function!begin name=<function name> param=parameters]
<content>
[!function!end]
[!procedure!begin name=<procedure name> param=parameters]
<content>
[!procedure!end]
!devel-end!

!devel settings caption=Configuration options for the algorithm command.
                module=MooseDocs.extensions.algorithm
                object=FunctionComponent

### Loop Commands

!devel! example caption=Example of using loop line commands
!algorithm
[!while!begin condition=<condition>]
<content>
[!while!end]
[!for!begin condition=<condition>]
<content>
[!for!end]
!devel-end!

!devel settings caption=Configuration options for the algorithm command.
                module=MooseDocs.extensions.algorithm
                object=LoopComponent

### If-Then Commands

!devel! example caption=Example of using if-then line commands
!algorithm
[!ifthen!if condition=<condition>]
<content>
[!ifthen!elif condition=<condition>]
<content>
[!ifthen!else]
<content>
[!ifthen!end]
!devel-end!

!devel settings caption=Configuration options for the algorithm command.
                module=MooseDocs.extensions.algorithm
                object=IfThenComponent

### Statement Command

!devel! example caption=Example of using statement line commands
!algorithm
[!state text=<text>]
!devel-end!

!devel settings caption=Configuration options for the algorithm command.
                module=MooseDocs.extensions.algorithm
                object=StatementComponent

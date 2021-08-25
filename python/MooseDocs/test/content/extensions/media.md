# Media Extension

## Images

### Outside Float

!media Flag_of_Michigan.svg
       latex_src=Flag_of_Michigan.pdf
       style=width:75%;text-align:center;

### Inside Float

!media Flag_of_Washington.svg
       style=width:75%;text-align:center;
       latex_src=Flag_of_Washington.pdf
       caption=Image float with cation

!media Flag_of_Montana.svg
       latex_src=Flag_of_Montana.pdf
       id=montana-flag
       caption=Image float with caption and label
       style=width:75%;

!media Flag_of_New_York.svg
       latex_src=Flag_of_New_York.pdf
       id=newyork-flag
       caption=Image float centered
       style=width:75%;text-align:center;

!media Flag_of_Idaho.svg
       latex_src=Flag_of_Idaho.pdf
       prefix=Flag
       caption=Image float with custom prefix
       id=idaho-flag
       style=width:75%;text-align:center;

### Links to Images

Montana: [montana-flag]

New York: [newyork-flag]

Idaho: [idaho-flag]


### Width

!media Flag_of_Michigan.svg
       latex_src=Flag_of_Michigan.pdf
       style=width:15%;text-align:center;

!media Flag_of_Michigan.svg
       latex_src=Flag_of_Michigan.pdf
       caption=Width is set to 15%
       style=width:15%;

## Videos

!media https://upload.wikimedia.org/wikipedia/commons/c/c0/Big_Buck_Bunny_4K.webm
       poster=big_buck_bunny.jpg
       latex_src=big_buck_bunny.jpg
       style=width:80%;display:block;margin-left:auto;margin-right:auto;

!media https://upload.wikimedia.org/wikipedia/commons/c/c0/Big_Buck_Bunny_4K.webm
       id=big-bug-bunny
       poster=big_buck_bunny.jpg
       latex_src=big_buck_bunny.jpg
       style=width:80%;
       caption=Big Bug Bunny is an open-source movie short.

!media http://www.youtube.com/embed/2tJwBsYaLaI
       id=training-webinar
       latex_src=training-webinar.png
       caption=MOOSE training webinar given on June 9--10, 2020.

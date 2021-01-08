!alert construction
This tutorial is incomplete, but feel free to browse the currently available content. Otherwise, refer back to the [examples_and_tutorials/index.md] page for other helpful training materials or check out the MOOSE [application_development/index.md] pages for more information.

# Tutorial 1: Application Development id=tutorial-1

In this tutorial, the reader shall work through the steps to create a custom MOOSE-based application to solve real-world physics problems. The instructions begin with defining the problem and converting the governing [!ac](PDEs) into an expression compatible with MOOSE. As the tutorial progresses, the core C++ classes that are available to developers for solving problems, as well as the basic systems of MOOSE, will be discussed.

This tutorial is the focus of the live hosted [examples_and_tutorials/index.md#workshop]. New users are encouraged to engage themselves in this training by +reading the content carefully and reproducing the steps, without simply copying and pasting code+. This tutorial is designed to be an in-depth explanation of creating a complete, custom multiphysics application including the process of using a repository and testing.

!media tutorial01_app_development/moose_intro.png
       style=width:80%;display:block;margin-left:auto;margin-right:auto;
<!--Delete this image from `large_media/` or just use it here? I think it looks kind of nice here-->

## Tutorial Contents id=contents

!content outline location=getting_started/examples_and_tutorials/tutorial01_app_development
                 max_level=6
                 hide=tutorial-1 contents
                 no_prefix=1
                 no_count=preface problem-statement

!content pagination next=tutorial01_app_development/preface.md

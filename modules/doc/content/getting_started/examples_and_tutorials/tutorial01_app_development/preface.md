!content pagination previous=tutorial01_app_development/index.md
                    next=tutorial01_app_development/problem_statement.md
                    margin-bottom=0px

# Preface

## Navigation

Jump around page sections by using the header links provided on the right table:

!media tutorial01_app_development/scrollspy.png
       style=width:25%;display:block;height:auto;margin-left:auto;margin-right:auto;

Flip through pages with the "[!icon!arrow_back] +Previous+" and "+Next+ [!icon!arrow_forward]" buttons located at the start and end of the page:

!media tutorial01_app_development/pagination.png
       style=width:84%;display:block;margin-left:auto;margin-right:auto;

Head back to the index page by following the "=tutorial01_app_development=" link located at the top of the page:

!media tutorial01_app_development/breadcrumbs.png
       style=width:100%;display:block;margin-left:auto;margin-right:auto;

Go to any section of the tutorial from the [tutorial01_app_development/index.md#contents] section of the index page:

!media tutorial01_app_development/outline.png
       style=width:61%;display:block;margin-left:auto;margin-right:auto;

!!!
NOTE: These screenshots were captured with the Shutter application (shutter-project.org)

The borders and shadow were then produced by running the following shell script:

```
convert -border 8 -bordercolor 'rgb(0,88,151)' $1 $2

convert $2 \( +clone -background '#7A7777' -shadow 88x5+0+5 \) +swap \
	-background none -layers merge +repage $2
```

The images were then scaled to the page width based on the widest image resolution (breadcrumbs.png)

TODO: a nice border/shadow tool for MooseDocs media would be nice
!!!

## Acronyms

!acronym list location=getting_started/examples_and_tutorials/tutorial01_app_development

!content pagination previous=tutorial01_app_development/index.md
                    next=tutorial01_app_development/problem_statement.md

# Content Extension

## Table of Contents id=table-of-contents

!content toc levels=2 3 columns=2 hide=table-of-contents

## A-to-Z Index

The following is a complete list of all pages.

!content a-to-z level=3

## Page List

The following is a list of markdown pages within the "extensions" directory.

!content location=extensions level=3

## Some Content for TOC

### TOC 1

### TOC 2

## Outline Pages id=outline-pages

The following is a list of all headings in the "extensions" directory, except this section's heading, in the order in which they appear on the [main index page](/).

!content outline max_level=6
                 hide=outline-pages
                 pages=core.md
                       autolink.md
                       include.md
                       style.md
                       media.md
                       gallery.md
                       listing.md
                       table.md
                       devel.md
                       package.md
                       alert.md
                       katex.md
                       bibtex.md
                       common.md
                       layout.md
                       materialicon.md
                       content.md
                       config.md
                       graph.md
                       navigation.md
                       appsyntax.md
                       template.md
                       civet.md
                       sqa.md
                       acronym.md

!alert note title=Rebuilding the Outline
If new headings are created or deleted during a live serve, the outline won't update until rebuilding. Although, for the case of a deleted header, its link will stop working. It will also start working again if it is deleted and then recreated.

*TODO: find a way to check, in content.py, RenderContentOutline(), that a heading link actually works. For the case where it doesn't (because it was deleted during a live serve), remove it from the list, i.e., don't add it in the first place.*

*Also, find a way to handle newly created headings - this would be much more tricky, since the complete heading list remains static following the initial build. For now, the simplest way to handle creation/deletion is to just re-serve.*

## Outline Directory id=outline-directory

The following is a list of all level 1 & 2 headings in the "extensions" directory and its subdirectories, except this section's heading, in alphabetical order of the names of MooseDown files which create those headings.

!content outline location=extensions/ recursive=True max_level=2 hide=outline-directory

## Next/Previous (Pagination)

"Next" and "Previous" buttons are designed to be the last element of a page. However, for the case where more markdown content is added afterwards, the same margin is enforced below the buttons as are above. They need to appear as items which are distinct from page's main body, similar to how the breadcrumbs navigation is separated, and so this why the margins are enforced.

The following buttons were configured to link to those pages which appear before and after this one on the [main index page](/).

!content pagination previous=materialicon.md next=config.md

### Buttons with Page Titles

The following buttons were configured to link to those pages which appear before and after this one on the [main index page](/) and to use those page's title as their text.

!content pagination previous=materialicon.md next=config.md use_title=True

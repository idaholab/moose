# Core Extension

The core extension is the portion of the MooseDocs language that is designed to mimic [markdown]
syntax. MooseDown is far more strict than traditional [markdown] implementations.
Therefore, the following sections should be read in detail to understand the supported syntax,
especially if you are familiar with more general markdown formats.

Syntax is separated into two groups: [block](#core-block) and [inline](#core-inline). Block
content is made of items that make, as the name suggests, blocks of text. Blocks of
text include items such as code snippets, quotations, and paragraphs themselves. On the other hand,
inline items are those that are applicable to small portions of text (e.g., words). Bold and
italics are two examples of inline items.

!alert note title=MooseDown is a restricted version of [markdown].
To unify content and to create a fast parser a strict, limited set of markdown is being used to
define the MooseDocs syntax. The following sections detail the syntax.

## Settings id=settings

In general, most block level syntax and some inline syntax accepts key-value pair settings. Where
the settings appear within the syntax varies and is detailed in the following sections.
[settings-example] show how the settings are specified in [#links].
However, the settings are applied in a uniform manner. Foremost, the key and value are separated by
an equal (`=`) sign +without spaces+ surrounding. The value may contain spaces, with the space after
the equal sign being the exception.

If syntax has settings then the key-value pairs, the default value (if any), and a short description
will be provided in a table. For example, [code-settings] lists the available settings
for the fenced code blocks discussed in the [#fenced-code] section.

!devel example id=settings-example
               caption=Example use of settings within [#links]. Settings are key-value pairs that
                       are specified with a separating equal sign (no spaces exist on either side).
[google](https://www.google.com style=color:teal;)

## Block Syntax id=core-block

Block level content, as the name suggests, are blocks of text. In all cases, blocks must
begin and end with empty lines (with the exception of the start and end of the file). This
restriction allows for special characters such as the hash (`#`) to be used at the start
of a line without conflicting with heading creation (see [#headings]). Additionally, this
allows content and settings to span multiple lines.

### Code id=fenced-code

Code blocks or snippets---as shown in [fenced-code-example]---are created by enclosing the code for
display in triple back-ticks (```), this is commonly referred to as fenced code blocks. Two
requirements exist for creating the code blocks:

1. the back-ticks must be preceded by an empty line and
1. the back-ticks must start at the beginning of a line.

Settings for code blocks are defined by key-value pairings that follow the back-ticks;
[code-settings] lists the available settings for code blocks.

!devel example caption=Basic fenced code block. id=fenced-code-example
```language=bash
export METHOD=opt
```

!devel settings module=MooseDocs.extensions.core object=CodeBlock id=code-settings caption=Available settings for fenced code blocks.

### Quotations

Quotation blocks are created by starting a line with the `>` character, with a single trailing
space as shown in [quote-example]. Then each additional line that belongs within the quotation
must also begin with a `>` character. Within the quotations any valid markdown is acceptable,
as shown in [quote-nested-example].

!devel example caption=Basic block quote. id=quote-example
> This is a quotation.

!devel example caption=Nested content in block quotes. id=quote-nested-example
> Quotations begin with the `>` character and may
> contain any valid markdown content, include quotes and code as shown here.
>
> > This begins another quotation, which also contains a fenced code block.
> >
> > ```language=python
> > for i in range(10):
> >   print i
> > ```
>
> Since quotations are block content they must end with an empty line,
> therefore the nested quote above must contain an empty line.

### Headings id=headings

Headings can range from level one to six and are specified using the hash (`#`) character, where the
number of hashes indicate the heading level (see [heading-basic-example]). The following is required
to define a heading:

1. the hash(es) must be followed by a single space,
1. the hash(es) must +not+ be preceded by a space.


Settings, as listed in [heading-settings], are applied after the heading title text as shown in
[heading-standard]. Headings may also span multiple lines as shown in [heading-multiline].

!devel! example caption=Basic use of all six heading levels. id=heading-basic-example
# Level One

## Level Two

### Level Three

#### Level Four

##### Level Five

###### Level Six

!devel-end!

!devel example caption=Use of settings within heading. id=heading-standard
## Level Two style=font-size:75pt;color:red; id=level-two

!devel example caption=Use of settings within heading. id=heading-multiline
## A Heading May Span
   Multiple Lines (this is useful if they are really long)
   style=font-size:15pt
   id=level-two

!devel settings module=MooseDocs.extensions.core object=HeadingBlock caption=Available settings for headings. id=heading-settings

### Unordered List id=unordered

Unordered list items in MooseDown +must+ begin with a dash (`-`), as shown below in
[unordered-basic-example]. As with any block item, a list must be preceded and followed by an empty
line. However, lists have additional behavior that allow for nested content to be included.

1. An empty line will not stop the list if the following line begins with another list marker
   (i.e., `-`), in this case the list continues.

1. An empty line followed by a non-list marker---everything except a hyphen---will stop the
   list. Otherwise, a list will stop if +two+ empty lines are encountered, otherwise it will
   continue to add items to the current list.

List items may contain lists, code, or any other markdown content and the item content may
span many lines. The continuation is specified by indenting the content to be included within the
item by two spaces, as shown in [unordered-nested-example].

!devel! example caption=Unordered list basic syntax. id=unordered-basic-example
- Item 1
- Item 2
!devel-end!

!devel! example caption=Lists can contain other markdown content. id=unordered-nested-example
- Item with code
  Content can be contained within a list, all valid MooseDown syntax can be used.

  ```
  int combo = 12345;
  ```

- Another item


- Foo

  ```
  bar
  ```
!devel-end!

As mentioned above, lists can contain lists, which can contain lists, etc. A sub-list, which is a
list in a list, is created by indenting at the level of the list item within which it
should be contained. Since lists are block items, it must be begin and end with empty lines. And, since
this is a list it also follows the aforementioned rules for list continuation.
[unordered-sublist-example] demonstrates the syntax for creating nested lists.

!devel! example caption=Nested unordered lists. id=unordered-sublist-example
- A
- B

  - B.1
  - B.2

    - B.2.1
    - B.2.2

  - B.3

    - B.3.1

      - B 3.1.1

    - B.3.2

  - B.4

- D
!devel-end!


### Ordered List

A numbered list works nearly identically to unordered lists (see [#unordered]), but starting with a
number followed by a period and a single space. The number used for the first item in the list
will be the number used for the start of the list, see [ordered-example].

!devel! example id=ordered-example caption=Example of ordered lists with a starting number and a second with nested content.
42. Foo
1. Bar


1. Another list that contains nested content.

   1. Ordered lists can be nested and contain markdown.

      ```
      code
      ```

!devel-end!

### Shortcuts id=shortcuts

It is possible to define shortcuts for use within your document via a [shortcut link](#shortcut-link). The shortcuts
are defined using traditional markdown syntax as in [#shortcut-link]. However, these are block
items, so to maintain consistent behavior they must be surrounded by blank lines.

!devel! example id=shortcut-example caption=Markdown shortcut definition.
You can create shortcuts to common items: [foo].

[foo]: bar

!devel-end!


## Inline Content id=core-inline

Inline content comprises formatting, as the name suggests, that occurs within lines of text.
Examples include inline code and text formatting such as +bold+.

### Monospace (Inline Code) id=monospace

`Monospaced` text is specified by encasing the text with single  back-ticks, the content within
the back-ticks is reproduced verbatim, thus allowing for MooseDocs syntax to be enclosed within
the back-ticks, as in [monospace-example].

!devel! example id=monospace-example caption=Example of monospaced text.
The following MooseDocs `[google]` will show verbatim because it
is enclosed within back-ticks, without the back-ticks [google] creates a link.

[google]: https://www.google.com
!devel-end!

### Text Formatting

Inline text formatting differs in MooseDocs from traditional [markdown] in many ways, the reasons
for the differences include avoiding reusing the same symbol (i.e., `*`) for multiple formats,
making the syntax suitable to simple parsing of multiline regions, and to support a wide range of
formats.

For all formats the starting character must be immediately followed by a non-whitespace
character. The starting character must also be preceded by a whitespace, except in the case of the
super and subscript which must be preceded by a non-whitespace character. The ending character must
be immediately preceded by a non-whitespace character.  The following table lists the available
formats with start and end characters.

| Name | Character | MooseDown | Result (HTML) |
| - | - | - | - |
| Underline | `=` | `=underline=` | =underline= |
| Strong | `+` | `+strong+` | +strong+ |
| Emphasis | `*` | `*emphasis*` | *emphasis* |
| Strikethrough | `~` | `~strikethrough~` | ~strikethrough~ |
| Superscript | `^` | `foo^bar^` | foo^bar^ |
| Subscript | `@` | `foo@bar@` | foo@bar@ |

The formatting can be arbitrarily nested and span multiple lines within the same paragraph, thus
as shown in [format-example], complicated and compound formatting of text is possible.

!devel example id=format-example caption=Example inline text formatting in MooseDocs
Yo, dawg I heard you like formatting, so I created =underline
formatted text that contains text with ~strikethrough that
contains +bold formatting with *emphasis that has some^superscript
text with a@subscript@^*+~=, I hope you like it.

### Links id=links

MooseDocs uses traditional [markdown] syntax for links; however, it also supports settings within
the link (see [settings-example]). The settings are the expected key-value pairings common to much of the MooseDocs
syntax. The available settings for links is include in [link-settings].

!devel settings module=MooseDocs.extensions.core id=link-settings object=LinkInline caption=Available settings for links.

### Shortcut links id=shortcut-link

Links to shortcuts use the typical [markdown] syntax of a key enclosed in square brackets (`[key]`),
where the key references a shortcut, which are defined in the [#shortcuts] section. Refer
to [shortcut-example] for a demonstration of shortcut and shortcut link use.

In addition to linking to [#shortcuts] created directly, the same syntax is used to reference
headings that have an "id" setting applied, see [heading-link-example]. When used in this fashion
the heading text directly replaces the shortcut link and a link to the heading is created.

!devel! example id=heading-link-example caption=Example showing use of shortcut links to reference a heading.
Using shortcut link syntax, it is possible to link to [#heading].

# A Heading with Link id=heading

!devel-end!

### Punctuation

When rendering HTML, MooseDocs converts the following punctuation to the correct symbols, the
table below lists the conversions that are performed.

| MooseDocs | HTML |
| :- | :- |
| `--` | `&ndash` |
| `---` | `&mdash` |

[markdown]: https://en.wikipedia.org/wiki/Markdown

### Line Breaks

Line breaks can be forced by using `\\` within the text followed by a space or the end of a line,
as in [break].

!devel! example id=break caption=Example line break.
This sentence has a\\ line break. And so does\\
this.
!devel-end!

### Escape Characters

In some cases the use of characters such as a bracket or exclamation are needed in a context that
is recognized as special markdown syntax. In this case, the character should be escaped, using the
`\` character, as in the following examples.

| MooseDocs | HTML |
| :-        | :-   |
| `\!`      | \!   |
| `\[`      | \[   |
| `\]`      | \]   |
| `\@`      | \@   |
| `\^`      | \^   |
| `\*`      | \*   |
| `\+`      | \+   |
| `\~`      | \~   |
| `\-`      | \-   |

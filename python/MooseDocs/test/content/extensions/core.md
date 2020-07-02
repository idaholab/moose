# Core Extension

## Code Blocks

```
text
```

```cpp
int y;
```

```language=python
for i in range(10):
  print i
```

## Quotations

### Multiline Quotation

> Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore
> et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut
> aliquip ex ea commodo consequat. +Duis+ aute irure dolor in reprehenderit in voluptate velit esse
> cillum dolore eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non proident, sunt in
> culpa qui officia deserunt mollit anim id est laborum.

### Nested Quotations and Lists

> This is a quotation that contains a quotation.
>
> > This should be a nested quotation.
>
> - A list.
> - With two items
>
>   > The second contains a quote as well, which has a numbered list.
>   >
>   > 1. one
>   > 2. two

## Headings

# One

## Two

### Three

#### Four

##### Five

###### Six

## Heading with Style style=color:red;

## Unordered Lists

### Single level lists id=unordered-single-level-lists

- Item 1
- Item 2
- Item 3
  The third items has some content, this content contains paragraphs.

  This is a second paragraph in the list.

- Item 4 should be apart of the same list.



- Item 1b: This should start a new list, because of the two blank lines.
- Item 2b



### Nested lists id=unordered-nested-lists

- Item 1

  - Item 1.1
  - Item 1.2

- Item 2

  - Item 2.1

    - Item 2.1.1
    - Item 2.1.2

      > A quotation.

  - Item 2.2

- Item 3


## Ordered List

### Single level lists id=ordered-single-level-lists

1. One
1. Two
1. Three
   Just like above, this should contain two paragraphs.

   This is a new paragraph.

1. Four

### Starting number

2. Two
2. Three


42. Forty-two
42. Forty-three


12345. Twelve thousand *three* hundred and forty-five
12345. Twelve thousand three hundred and forty-six

       1. A nested item within a +huge+ number.

### Nested lists id=ordered-nested-lists

1. One

   1. 1.1
   2. 1.2
   3. 1.3

2. Two

   2. 2.1
   2. 2.2

      2. ~2.2.1~
      2. 2.2.2
      2. 2.2.3

         > A quote.
         >
         > > Go deeper.
         > >
         > > 54. Fifty-four

## Shortcuts and Shortcut links.

A popular search engine is [google].

[google]: https://www.google.com

## Inline formatting

super^script^ not super ^script^

sub@script@ not sub @script@

=underline=  = not underlined=

*emphasis* * not emphasized*

+bold+ + not bold+

~strike~ ~ not strike~

`mono` ` not mono`

Yo, dawg I heard you like formatting, so I created =underline
formatted text that contains text with ~stikethrough that
contains +bold formatting with *emphasis that has some^superscript
text with a@subscript@^*+~=, I hope you like it.

## Links id=links

This is a link to [google](https://www.google.com).

#### Skip the level

This heading is to testing

# Comment Extension

The comment extension provides a mechanism to make comments within markdown text that will not be
rendered in the final MooseDocs output. The available configuration items for this extension are
provided in [comment-config].

!devel settings module=MooseDocs.extensions.comment
                object=CommentExtension
                id=comment-config
                caption=Configuration items for the comment extension.

## Inline Comments

In order to create inline, or single-line, comments, the `!!` syntax is available. An example of
this is shown below in [inline-example].

!devel! example id=inline-example caption=Example usage of inline MooseDocs comments

This is regular text.

!! This is an inline comment.

!devel-end!

## Block Comments

In order to create block, or multi-line, comments, the `!!!` syntax is available. An example of this
is shown below in [block-example].

!devel! example id=block-example caption=Example usage of block MooseDocs comments

This is regular text.

!!!
This is a block comment.
Multiple lines can exist within them.
!!!

!devel-end!

!alert! warning title=HTML-style block comments are not supported
HTML-style comments with the following syntax:

!!!
NOTE: there is an intentional space at the beginning of this example, in order to avoid the regex
associated with flagging HTML-style syntax.
!!!

```
 <!-- This is an insightful comment. -->
```

were previously allowed in MooseDown, but were deprecated in 2019 and finally removed in July 2025.
The inline and block comment syntax above should be used going forward.
!alert-end!

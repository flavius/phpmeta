README
======

Meta is a PHP extension module aimed at building the AST representation
of PHP source code files.

Meta is a prototype, I am still doing research to discover good ways
of implementing it.

Meta is going to fill the gap between Reflection, runkit and the tokenizer.


What is a syntax tree?
----------------------
A syntax tree is the canonical representation of a source code. The code
is parsed according to the tokens' [associativity](http://php.net/manual/en/language.operators.precedence.php)
and precedence, and together they make a tree. For instance, the tree of the code:

    The answer is <?php echo 20+22;

could look like this:

![A syntax tree](http://i.imgur.com/OF6gO.png)

The things in black are all the nodes of this syntax tree. At the top there is
the root of the tree, similar to a DOMDocument. It has two children, the inline
text, and a new node holding the elements inside the PHP processing.

The blue arrows show the parent-child relationship.

The nodes with a yellow background could be left out, and we could still reconstruct
a valid PHP source equivalent to the original PHP code, based on logical assumptions
from the context in which these nodes occur.

All the nodes, the required ones ('+', 20, 22, 'The answer is ') and the optional
ones ('<?php', ' ' and ';') form the *concrete syntax tree* of the source code.
Only the required ones form the *abstract syntax tree* of the code. Meta is able
to produce both, ASTs and the more verbose CSTs, depending on the flags you
pass to it.

The black boxes (ASTTree and ASTNodeList) are "containers" and they can contain
an infinite number of children.

Current status
==============

Currently, work is being done on constructing the AST tree programatically.

See a sample code in `tests/ASTUnaryNode_basic.phpt`.

To be done:

  * introduce classes for all language constructs out there (work has begun in
  `parser.c`)
  * make the scanner recognize all tokens (source: `php_scanner.re`)
  * make the parser parse any valid source code (source: `php_parser.y`)
  * introduce a non-obtrusive meta-parsing hook; this could be easily
  done by putting `#@myhook`. When the node following after this "meta instruction"
  is reduced, the hook is called, getting as parameter the AST node which has
  just been reduced (**A**)
    * alternative 1: the ASTTree holds a map of AST classes, if the user wishes
  to hook his code, he can extend ASTTree and the target AST node's classes,
  and a method like `ASTNode::reduce()` could be called (**B**)
    * alternative 2: the ASTTree receives as parameter a map of XPath-like
  expressions and callbacks. At reduction time, when an "ASTPath" is matched,
  the appropiate callback is called (**C**)

If and how A, B, C will be done, is subject to RFC. Any input is welcome.

Already done:

  * most of the work has been done by sitting in front of the whiteboard :-)

What it can be used for
=======================

  * meta programming
  * code preprocessing
  * pretty printing
  * static code analysis (and everything that can be done through it, QA, semi-automatic security and logical bugs detection)

Meta itself will not be able to do these things by itself, but it will provide
the necessary infrastructure to userland scripts for specialized uses like
the ones mentioned above.

How to use it
=============

Please see the `tests/` to get an idea of how it could be used.
Keep in mind this is a work in progress, a prototype. The code
is rough and there are many TODOs (many of them documented though).

Internals
=========

Meta is using re2c as the lexer generator and a patched version of lemon
for the parser.

The extension is split in two, the scanner and the parser, each made
of two more components: the internal part and the runtime part.

`meta_scanner.{c,h}` and `meta_parser.{c,h}` contain the internal code,
not exposed directly to the runtime. The .c files are generated by
re2c and lemon from their .re and .y counterparts. Lemon and re2c
also generate two more headers, the `_defs.h` ones, which you
should not include directly, but through the `.h` files mentioned above.

`scanner.c` and `parser.c` contain the functionalities exported to the
runtime, through `meta.c`.

The ZE2's internal lexer could not be used because it is made for speed
and minimal memory requirements. As such, it strips out information
from the scanned code, information which meta needs to achieve its goals.

By keeping this information, tools using this extension can even detect
coding standards violations (e.g. "echo" vs. "ecHo").

Via appropiate flags though, the user will be able to tell it to minimize
memory usage by skipping unneeded tokens (e.g. you don't care about whitespaces
or capitalization if you are analyzing a code for security holes).

Also:

  * ZE2 is bound to the php.ini settings (`CG`), meta will be able to
parse any code (again, you can fine-grain the process via appropiate flags),
regardless of the local settings of the system.
  * one main priority is to be able to analyze PHP code compatible
with different versions of PHP, regardless of the version running the
extension itself.

Meta is supposed to work as a PHP 5.3+ extension and 
interpret PHP 5.3+ code.

Roadmap
=======

With the 0.0.1 milestone ready, the user will be able to construct and
modify the AST tree of simple math expressions and statements.

As such, version 0.0.1 will be feature-complete, but only for a small
subset of the language.

Coding style guidelines
=======================

(strongly inspired from the transcode coding style)


Syntax
------

* Lines: must not be longer than 79 columns, and should be kept to 75
  columns or less when feasible.

* Indentation: must use the basic unit of four columns.

* Tab characters: (ASCII 0x09) must not be used for indentation or alignment
  of source code, and should be avoided in other cases (such as in strings)
  where possible.  Tabs should not be assumed to have any particular width.

* Comments: the multiline comments should use standard C style (/*...*/); however,
  C++ style (//) an be used for inline comments (comments placed on the same
  line as code). Comments on their own lines should start at the same indentation
  level as the surrounding code; inline comments should, if feasible, line
  up with other nearby inline comments.  Multi-line comments should begin
  with a single "*" on the second and subsequent lines, aligned with the
  "*" in the "/*" on the first line; the final "*/" should be placed on the
  same line as the last line of comment text, unless doing so would cause
  that line to exceed 79 characters.  However, for lengthy comments it is
  also acceptable to place the initial "/*" and trailing "*/" on their own
  lines.  Examples:

      /*
       * This is a lengthy multiline comment, perhaps a function header,
       * with the comment delimiters placed on their own lines.
       */
      void myfunction()
      {
          int var1;          // Inline comment about var1
          double variable2;  // Inline comment about var2

          /* This is a shorter multiline comment about what the next bit of
           * code will do. */
          first();
          /* This is a single-line comment */
          second();
      }

* Documentation: every function should have a header in doxygen format
  which describes the purpose of the function.

* Statements: only one per line.  Statements (including the empty
  statement ";") may not be written on the same line as a control keyword
  (if, for, or while).

* Spaces: they are placed between binary operators and their operands, except
  for the member-reference operators "->" and "."; spaces are not placed
  between unary operators and their operands.  However, spaces may be
  omitted around binary operators when doing so would improve readability.
  Examples:

      i = j * 2;
      structure->field++;
      result += i*60 + (j+59)/60;

* Parentheses: they are required when && and || (or two or more of &, |, and ^)
  are used in the same expression to explicitly indicate the order of
  evaluation; do not rely on the default order of evaluation.

* Parentheses: parentheses following the control statements "if", "for", and
  "while" are preceded by a space.  Parentheses following function or macro
  calls are not preceded by a space.  Example:

      if (flag) {
          function();
      }

* Spaces: placed spacing after the comma of each parameter to a function or
  macro; however, they may be omitted in function calls which are
  themselves parameters to a function or macro, or when including them
  would make the line exceed 79 columns in length (and removing them does
  not harm the readability of the code).  Spaces are also placed after the
  semicolons in a for statement.  Spaces are not placed after the opening
  parenthesis or before the closing parenthesis of a function/macro call or
  control statement.  Examples:

      function(param1, param2);
      function(param1, strchr(string,'/'), param3);
      for (i = 0; i < count; i++) ...

* Braces: the opening braces of control statement blocks go on the same line
  as the control statement (if, for, etc.) associated with the block; function
  blocks and "naked blocks" (those not associated with any control
  statement or function) have the opening brace alone on a line, indented
  to the same level as the surrounding code.  Closing braces are indented
  to the same level as the line containing the opening brace.  Examples:

      if (flag) {
          /* indented code goes here */
          ...
      }

      int function(int param1, char *param2)
      {
          /* indented code goes here */
          ...
          /* start of a naked block */
          {
              int foo;
              ...
          }
      }

* Statements: if they are longer than one line, it should be broken at
  logical operators (&& or ||); the operator should be placed at the
  beginning of the next line, and should be indented to show which term the
  operator belongs with.  The closing parenthesis for the if statement and
  the subsequent open brace should be alone on a line, aligned with the
  first line of the if statement.  (Braces are required in this case.)
  Conditions for for and while statements should never span multiple lines,
  though a for statement may be broken at the semicolons if necessary.
  Examples:

      if (ok && (strcmp(option, "option-name-1") == 0
                 || strcmp(option, "option-name-2") == 0)
      ) {
          ...
      }

      if (!init_first()
       || !init_second()
       || !init_third()
       || !init_fourth()
      ) {
          /* This example outdents the || by three spaces
           * to make the terms line up. */
          ...
      }

      for (node = first_node(list, LISTTYPE_MYLIST); node != NULL;
           node = next_node(list, LISTTYPE_MYLIST))
      ) {
          ...
      }

* Else statement: if it is followed by an if is considered to be a single keyword,
  "else if". The if part should always be placed on the same line as and immediately
  following the else; the else if should never be split up onto two lines,
  nor should braces be used around the if block.  The exception to this is
  when the else and if refer to two conceptually distinct sets of
  conditions, in which case the if block should be enclosed by braces and
  indented.  Example:

      res = check_password( /* ... */ );
      if (res == 0) {
          ...
      } else if (res == 1) {
          /* "else if" on a single line */
          ...
      } else {
          if (otherflag) {
              /* "if" condition is different from "else"
               * condition, thus separate */
              ...
          }
      }

* Braces: braces are always placed around the body of a control statement
  (if, for, etc.). Examples:

      for (i = 0; i < count; i++) {
          function(i);
      }

      while (!done) {
          /* Braces required because of the nested "if" */
          if (do_stuff()) {
              done = 1;
          }
      }

      if (state == 0) {
          a = b;
      } else if (state == 1) {
          /* Every if/else body gets braces because this body
           * has two statements */
          b += a;
          a = 0;
      } else {
          state = 0;
      }

* Case labels: for a switch should be indented half of a normal indentation
  unit (two columns) from the line containing the switch with which they
  are associated; statements associated with a case should be indented a
  full unit from the line containing the switch (half a unit from the
  case).  If a case requires its own block, such as when it declares its
  own local variables, the opening brace is placed after the colon on the
  case line.  Example:
      switch (variable) {
        case 123: {
          int foo;
          ...
          break;
        }  /* case 123 */
        default:
          ...
          return -1;
      }

* Case: when a case in a switch block does not contain a break (or return)
  statement and deliberately "falls through" to the next case, a comment to
  this effect should be made at the bottom of the case.  Example:
      switch (state) {
        case 0:
          ...
          /* fall through */
        case 1:
          ...
          break;
      }

* Initializers: structured type initializers should use C99-style named
  initializers, to avoid bugs resulting from initialization of incorrect
  fields if the structure definition is changed.  This is particularly
  important if the structure definition is in a different source file,
  such as a header file.  Example:
      struct foo {
          int bar;
          double quux;
          int have_quux;
      };
      ...
      struct foo myfoo = {
          .bar = 1,
          .have_quux = 0,
      };


Semantics
---------

* Functions: they should not be overly long or complex; as a rule of thumb, if a
  function has more than four levels of indentation, or if it spans for more
  than 42 lines (topmost braces excluded), then it should probably
  be broken up into separate functions.

* Operators: do not use binary operators to perform arithmetic operations.
  In particular, never use bit-shift operators in place of multiplication or
  division by powers of two.  (Modern compilers are smart enough to convert
  arithmetic expressions like "x/2" to a bit-shift operation; moreover,
  right shift and division give different results on negative numbers.)
  Binary operators should only be used when operating on bitmasks or other
  values where there is a need to extract or modify particular bits,
  independent of the value of the variable as a whole.

* Operators: always use the increment and decrement operators (++ and --) as
  postfix operators, except in expressions that require prefix usage.

* Expressions: avoid nesting assignment expressions (including increment and
  decrement operations) within other expressions, to the extent that extracting
  the assignment operation would not unduly complicate the code.  In the
  following example, separating the strtok() call and its comparison would
  require changing the structure of the loop completely, so the example is
  acceptable as is:

      while ((s = strtok(NULL, " \t\r\n")) != NULL) {
          ...
      }

* Expressions: do not use an assignment expression as the condition in an
  "if", "while", or "for" test; explicitly compare the result of the assignment
  against 0 or NULL, as appropriate.

* Expressions: do not use the result of a logical expression (operators &&, ||, ==,
  etc.) as an operand in an arithmetic or binary expression.

* Pointers: always use NULL, not 0, in pointer comparisons.  For character
  comparisons, use 0 when checking for the null terminator at the end of a
  string; use '\0' only if the intent is to check for the ASCII character
  NUL as part of a string.  Do not use '\0' with variables declared as type
  int8_t or uint8_t, even though these types are usually equivalent to char
  and unsigned char.

* Expressions: the construct "!!expression" should not be used.  Use "expression != 0"
  (for characters or integers) or "expression != NULL" (for pointers)
  instead.

* Prototypes: all exported functions must be prototyped in a header file.

* Functions: when declaring a function that takes no parameters, use "(void)" instead
  of "()", since the latter does not declare a prototype in C.  Example:

      int function(void)  /* Function that takes no parameters */
      {
          ...
      }

* Functions: they should check all parameters for validity.  However, in cases
  where it is clear that a parameter is valid (for example, because the
  function is only called by one other function under specific
  circumstances), it is acceptable to assume the parameter is valid if that
  assumption is listed as a precondition in the function's header.

* Variables: they should be declared in a such a way they have the shortest scope
  as possible. Use freely mid-block C99 declarations.

* Goto: the goto statement should not be used except in error handling situations
  where it will help avoid multiple levels of if nesting or other awkward
  code.  Labels for goto should be outdented half of an indentation unit
  from the surrounding code (i.e., indented two columns less than the
  surrounding code).

* assert(): must not be used.  If a sanity check fails, the program should
  either recover as best it can or, if recovery is impossible, print a
  meaningful error message and exit gracefully.


Identifier naming
-----------------

* Globals: global variables (including variables declared static at the file level)
  and type names should use mixed upper- and lower-case, with an upper-case
  letter at the beginning of each distinct word in the name (including the
  first).

* Preprocessor: constants and macros should use all upper-case letters, with
  an underscore between distinct words in the name.

* Naming: structure names and local variables should use all lower-case letters,
  with an underscore between distinct words in the name.

* Naming: structure names for structures with an associated type name (i.e., from
  typedef) should be given the same name as the type, except with all
  letters in lowercase and followed by an underscore.  Example:
      typedef struct mytype_ MyType;
      struct mytype_ {
          MyType *next, *prev;
          ...
      };

* Naming: names should be descriptive.  For global variables, preprocessor macros
  and constants, type names, and structure names, names should generally
  consist of one or more full words.  For local variables, short names and
  abbreviations are permitted as long as it is clear what the variables are
  used for.  In general, one-letter local variable names should not be used
  other than the following:
      * c: character
      * f: file pointer (FILE *)
      * i, j, k: integers (usually counters)
      * n, p, q: integers (p may also be a pointer variable)
      * s: string
      * t: string or temporary variable
      * x, y, z: integers (usually position variables)


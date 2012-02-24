--TEST--
Construct an ASTTree from a given PHP source code
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
$tree = new ASTTree(0, "<?php 42+43; 10+11;");
//var_dump((string)$tree);
$tree->parse();
var_dump((string)$tree);
var_dump($tree);
?>
--EXPECT--
string(19) "<?php 42+43; 10+11;"
object(ASTTree)#1 (8) {
  ["root":protected]=>
  *RECURSION*
  ["parent":protected]=>
  NULL
  ["index":protected]=>
  int(-1)
  ["start_line":protected]=>
  int(0)
  ["end_line":protected]=>
  int(0)
  ["children":protected]=>
  array(1) {
    [0]=>
    object(ASTNodeList)#9 (6) {
      ["root":protected]=>
      *RECURSION*
      ["parent":protected]=>
      *RECURSION*
      ["index":protected]=>
      int(-1)
      ["start_line":protected]=>
      int(1)
      ["end_line":protected]=>
      int(0)
      ["children":protected]=>
      array(3) {
        [0]=>
        string(5) "<?php"
        [1]=>
        string(1) " "
        [2]=>
        object(ASTNodeList)#5 (6) {
          ["root":protected]=>
          *RECURSION*
          ["parent":protected]=>
          *RECURSION*
          ["index":protected]=>
          int(-1)
          ["start_line":protected]=>
          int(0)
          ["end_line":protected]=>
          int(0)
          ["children":protected]=>
          array(3) {
            [0]=>
            object(ASTBinaryNode)#4 (10) {
              ["type":protected]=>
              int(41)
              ["root":protected]=>
              *RECURSION*
              ["parent":protected]=>
              *RECURSION*
              ["index":protected]=>
              NULL
              ["start_line":protected]=>
              int(0)
              ["end_line":protected]=>
              int(0)
              ["fill":protected]=>
              array(1) {
                [2147483647]=>
                array(1) {
                  [0]=>
                  string(1) ";"
                }
              }
              ["lhs":protected]=>
              object(ASTUnaryNode)#2 (10) {
                ["type":protected]=>
                int(67)
                ["root":protected]=>
                *RECURSION*
                ["parent":protected]=>
                NULL
                ["index":protected]=>
                NULL
                ["start_line":protected]=>
                int(1)
                ["end_line":protected]=>
                int(1)
                ["fill":protected]=>
                array(0) {
                }
                ["operator":protected]=>
                NULL
                ["subtype":protected]=>
                int(0)
                ["operand":protected]=>
                int(42)
              }
              ["rhs":protected]=>
              object(ASTUnaryNode)#3 (10) {
                ["type":protected]=>
                int(67)
                ["root":protected]=>
                *RECURSION*
                ["parent":protected]=>
                NULL
                ["index":protected]=>
                NULL
                ["start_line":protected]=>
                int(1)
                ["end_line":protected]=>
                int(1)
                ["fill":protected]=>
                array(0) {
                }
                ["operator":protected]=>
                NULL
                ["subtype":protected]=>
                int(0)
                ["operand":protected]=>
                int(43)
              }
              ["operator":protected]=>
              string(1) "+"
            }
            [1]=>
            string(1) " "
            [2]=>
            object(ASTBinaryNode)#8 (10) {
              ["type":protected]=>
              int(41)
              ["root":protected]=>
              *RECURSION*
              ["parent":protected]=>
              *RECURSION*
              ["index":protected]=>
              NULL
              ["start_line":protected]=>
              int(0)
              ["end_line":protected]=>
              int(0)
              ["fill":protected]=>
              array(1) {
                [2147483647]=>
                array(1) {
                  [0]=>
                  string(1) ";"
                }
              }
              ["lhs":protected]=>
              object(ASTUnaryNode)#6 (10) {
                ["type":protected]=>
                int(67)
                ["root":protected]=>
                *RECURSION*
                ["parent":protected]=>
                NULL
                ["index":protected]=>
                NULL
                ["start_line":protected]=>
                int(1)
                ["end_line":protected]=>
                int(1)
                ["fill":protected]=>
                array(0) {
                }
                ["operator":protected]=>
                NULL
                ["subtype":protected]=>
                int(0)
                ["operand":protected]=>
                int(10)
              }
              ["rhs":protected]=>
              object(ASTUnaryNode)#7 (10) {
                ["type":protected]=>
                int(67)
                ["root":protected]=>
                *RECURSION*
                ["parent":protected]=>
                NULL
                ["index":protected]=>
                NULL
                ["start_line":protected]=>
                int(1)
                ["end_line":protected]=>
                int(1)
                ["fill":protected]=>
                array(0) {
                }
                ["operator":protected]=>
                NULL
                ["subtype":protected]=>
                int(0)
                ["operand":protected]=>
                int(11)
              }
              ["operator":protected]=>
              string(1) "+"
            }
          }
        }
      }
    }
  }
  ["source":protected]=>
  string(19) "<?php 42+43; 10+11;"
  ["flags":protected]=>
  int(0)
}

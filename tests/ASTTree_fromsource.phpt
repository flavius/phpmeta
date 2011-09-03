--TEST--
Construct an ASTTree from a given source
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
$tree = new ASTTree(0, "<?php 42");
var_dump((string)$tree);
$tree->parse();
var_dump($tree);
var_dump((string)$tree);
?>
--EXPECT--
string(0) ""
object(ASTTree)#1 (8) {
  ["root":protected]=>
  NULL
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
    object(ASTNodeList)#4 (6) {
      ["root":protected]=>
      *RECURSION*
      ["parent":protected]=>
      NULL
      ["index":protected]=>
      int(-1)
      ["start_line":protected]=>
      int(1)
      ["end_line":protected]=>
      int(1)
      ["children":protected]=>
      array(3) {
        [0]=>
        string(5) "<?php"
        [1]=>
        string(1) " "
        [2]=>
        object(ASTNodeList)#3 (6) {
          ["root":protected]=>
          *RECURSION*
          ["parent":protected]=>
          NULL
          ["index":protected]=>
          int(-1)
          ["start_line":protected]=>
          int(1)
          ["end_line":protected]=>
          int(1)
          ["children":protected]=>
          array(1) {
            [0]=>
            object(ASTUnaryNode)#2 (9) {
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
              ["operator":protected]=>
              NULL
              ["fill":protected]=>
              array(0) {
              }
              ["operand":protected]=>
              int(42)
            }
          }
        }
      }
    }
  }
  ["source":protected]=>
  string(8) "<?php 42"
  ["flags":protected]=>
  int(0)
}
string(8) "<?php 42"

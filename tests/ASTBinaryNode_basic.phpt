--TEST--
Construct a simple ASTBinaryNode inside a simple tree programmatically
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
$tree = new ASTTree(0);
//TODO have the '+' implicitly, based on 41
$lhs=20;
$node = new ASTBinaryNode(41, $tree, $lhs, 22, '+');
$tree->appendChild($node);
var_dump($tree);
var_dump((string)$tree);
?>
--EXPECT--
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
    object(ASTBinaryNode)#2 (10) {
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
      array(0) {
      }
      ["lhs":protected]=>
      int(20)
      ["rhs":protected]=>
      int(22)
      ["operator":protected]=>
      string(1) "+"
    }
  }
  ["source":protected]=>
  NULL
  ["flags":protected]=>
  int(0)
}
string(5) "20+22"

--TEST--
Construct a simple ASTUnaryNode inside a simple tree, programatically
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
$tree = new ASTTree(0);
//TODO 1: use META_UNARY_PREOPERATOR from parser.h instead of 1
//TODO 2: actually expect an 'echo' there without telling it 'echo' explicitly
//TODO 3: fix the ' ' issue for given types of nodes (a HT with callbacks perhaps?)
$node = new ASTUnaryNode($tree, 78, 42, 1, 'echo ');
$tree->appendChild($node);
var_dump($tree);
var_dump((string)$tree);
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
    object(ASTUnaryNode)#2 (10) {
      ["type":protected]=>
      int(78)
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
      ["operator":protected]=>
      string(5) "echo "
      ["subtype":protected]=>
      int(1)
      ["operand":protected]=>
      int(42)
    }
  }
  ["source":protected]=>
  NULL
  ["flags":protected]=>
  int(0)
}
string(7) "echo 42"

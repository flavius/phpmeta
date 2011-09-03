--TEST--
Construct a simple ASTUnaryNode inside a simple tree
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
$tree = new ASTTree(0);
//TODO: actually expect an 'echo' there without telling it 'echo' explicitly
//TODO fix the ' ' issue for given types of nodes (a HT with callbacks perhaps?)
$node = new ASTUnaryNode(78, $tree, 42, 'echo ');
$tree->appendChild($node);
var_dump($tree);
var_dump((string)$tree);
--EXPECT--
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
    object(ASTUnaryNode)#2 (9) {
      ["type":protected]=>
      int(78)
      ["root":protected]=>
      *RECURSION*
      ["parent":protected]=>
      NULL
      ["index":protected]=>
      NULL
      ["start_line":protected]=>
      int(0)
      ["end_line":protected]=>
      int(0)
      ["operator":protected]=>
      string(5) "echo "
      ["fill":protected]=>
      array(0) {
      }
      ["operand":protected]=>
      int(42)
    }
  }
  ["source":protected]=>
  int(0)
  ["flags":protected]=>
  int(0)
}
string(7) "echo 42"

--TEST--
Construct an empty ASTTree programmatically
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
$tree = new ASTTree(0, "empty");
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
  array(0) {
  }
  ["source":protected]=>
  string(5) "empty"
  ["flags":protected]=>
  int(0)
}
string(0) ""

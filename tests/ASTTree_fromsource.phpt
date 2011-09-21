--TEST--
Construct an ASTTree from a given PHP source code
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
//$src = "<?php 42";
//$src2=&$src;
$tree = new ASTTree(0, "<?php 42");
var_dump((string)$tree);
$tree->parse();
//TODO include this in the output
//var_dump($tree);
var_dump((string)$tree);
?>
--EXPECT--
string(0) ""
string(8) "<?php 42"

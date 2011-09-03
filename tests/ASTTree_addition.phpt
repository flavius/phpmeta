--TEST--
Construct an ASTTree from a given source
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
//majors: 130 133 67 41 67 0
$tree = new ASTTree(0, "<?php 20+22");
$tree->parse();
var_dump((string)$tree);
?>
--EXPECT--
string(11) "<?php 20+22"

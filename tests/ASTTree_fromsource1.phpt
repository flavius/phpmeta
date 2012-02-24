--TEST--
Construct an ASTTree from a given PHP source code
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
$tree = new ASTTree(0, "<?php 42 +  43   ;");
$tree->parse();
var_dump((string)$tree);
?>
--EXPECT--
string(18) "<?php 42 +  43   ;"

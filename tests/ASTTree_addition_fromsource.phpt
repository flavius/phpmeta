--TEST--
Binary node as an addition from source
--CREDITS--
Flavius Aspra <flavius@php.net>
--FILE--
<?php
//majors: 130 133 67 41 67 0
$tree = new ASTTree(0, "<?php 20+22;");
$tree->parse();
var_dump((string)$tree);
?>
--EXPECT--
string(12) "<?php 20+22;"

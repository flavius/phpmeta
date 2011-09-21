--TEST--
tokenize a simple PHP source code
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
$inputs = array(
    'hello world',
);

$input = 'hello world';
$scanner = meta_scanner_init($input, 0);
var_dump($input);
do {
    $ret = meta_scanner_get($scanner);
    var_dump($ret);
} while(NULL != $ret);
unset($scanner);
?>
--EXPECT--
string(11) "hello world"
array(5) {
  ["major"]=>
  int(148)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(11) "hello world"
}
array(5) {
  ["major"]=>
  int(0)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  NULL
}
NULL

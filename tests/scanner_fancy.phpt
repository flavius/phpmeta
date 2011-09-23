--TEST--
Tokenize some common edge cases
--CREDITS--
Flavius Aspra <flavius.as@gmail.com>
--FILE--
<?php
$inputs = array(
    "<?php \t \t\n\t42",
    'hello',
    'abc<?= 42',
    "<?php\n42",
    "<?php\n \n42",
    "<?php 41+42",
    'abc<?php 42',
    'a<?php ',
    //TODO include again, now excluded because of "memcpy - blocks overlap" file_get_contents(__DIR__ . '/assets/bininput.php')
    //TODO write a valgrind supression file if needed
);

foreach($inputs as $input) {
    $scanner = meta_scanner_init($input, 0);
    var_dump($input);
    do {
        $ret = meta_scanner_get($scanner);
        var_dump($ret);
    } while(NULL != $ret);
    unset($scanner);
}

?>
--EXPECT--
string(13) "<?php 	 	
	42"
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  string(6) " 	 	
	"
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(2)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  int(42)
}
array(5) {
  ["major"]=>
  int(0)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(2)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  NULL
}
NULL
string(5) "hello"
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
  string(5) "hello"
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
string(9) "abc<?= 42"
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
  string(9) "abc<?= 42"
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
string(8) "<?php
42"
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  string(1) "
"
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(2)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  int(42)
}
array(5) {
  ["major"]=>
  int(0)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(2)
  ["end_line"]=>
  int(2)
  ["minor"]=>
  NULL
}
NULL
string(10) "<?php
 
42"
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(3)
  ["minor"]=>
  string(3) "
 
"
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(3)
  ["end_line"]=>
  int(3)
  ["minor"]=>
  int(42)
}
array(5) {
  ["major"]=>
  int(0)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(3)
  ["end_line"]=>
  int(3)
  ["minor"]=>
  NULL
}
NULL
string(11) "<?php 41+42"
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(1) " "
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  int(41)
}
array(5) {
  ["major"]=>
  int(41)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(1) "+"
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  int(42)
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
string(11) "abc<?php 42"
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
  string(3) "abc"
}
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(1) " "
}
array(5) {
  ["major"]=>
  int(67)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  int(42)
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
string(7) "a<?php "
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
  string(1) "a"
}
array(5) {
  ["major"]=>
  int(130)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(5) "<?php"
}
array(5) {
  ["major"]=>
  int(133)
  ["dirty"]=>
  bool(false)
  ["start_line"]=>
  int(1)
  ["end_line"]=>
  int(1)
  ["minor"]=>
  string(1) " "
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

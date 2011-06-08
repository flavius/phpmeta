<?php
$inputs = array(
    'abc<?php 42',
    'a<?php 42?>b<? 43?>',
    'hello',
    'abc<?= 42',
    "<?php\n42",
    "<?php\n \n42",
    "<?php 41+42",
);
//$inputs[] = file_get_contents('bininput.php');

$last = TRUE;

if($last) {
    end($inputs);
    do_test(key($inputs), current($inputs));
    die();
}

foreach($inputs as $title => $input) {
    do_test($title, $input);
}

function do_test($title, $input) {
    echo "\t\t$title for: '$input'\n";
    echo "=======================================================\n";
    $scanner = meta_scanner_init($input, 0);
    meta_scanner_get($scanner);
    unset($scanner);
    echo PHP_EOL, PHP_EOL;
}

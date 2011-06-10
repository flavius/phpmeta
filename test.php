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
);
//$inputs[] = file_get_contents('bininput.php');


$last = FALSE;
$repeat = TRUE;

if($last === TRUE) {
    end($inputs);
    do_test(key($inputs), current($inputs), $repeat);
    die();
}
elseif(is_numeric($last)) {
    do_test($last, $inputs[$last], $repeat);
    die();
}

foreach($inputs as $title => $input) {
    do_test($title, $input, $repeat);
}

function do_test($title, $input, $repeat=0) {
    echo "\ttest $title for input: '$input'\n";
    echo "=======================================================\n";
    $scanner = meta_scanner_init($input, 0);
    do {
        $ret = meta_scanner_get($scanner);
        if(NULL == $ret) break;
        printf("%s (%d)\t(%d)\t%d-%d\t", str_pad(meta_scanner_token_name($ret['major']), 20),
            $ret['major'], (string)$ret['dirty'], $ret['start_line'],
            $ret['end_line'], $ret['minor']);
        var_dump($ret['minor']);
    } while($repeat);
    unset($scanner);
    echo PHP_EOL, PHP_EOL;
}

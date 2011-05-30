<?php
$inputs = array(
    'empty' => '',
    'html only' => 'hello world',
    'empty processing' => '<?php',
    'processing' => '<?php 42',
    'processing and inline html' => 'abc<?php 42',
);

foreach($inputs as $title => $input) {
    echo $title, PHP_EOL;
    echo "================================================", PHP_EOL;
    meta_test($input);
    echo PHP_EOL;
}

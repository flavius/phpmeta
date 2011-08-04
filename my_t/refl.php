<?php
class Foo {

    public function __construct() {
    }

}

$refl = new ReflectionClass('Foo');

$foo = $refl->newInstance('bar',42);


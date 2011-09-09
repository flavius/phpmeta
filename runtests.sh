#!/bin/bash

PHP_V=5.3.6

PHP_D=$(echo $PHP_V | sed s/\\./_/g)

echo $PHP_D

sudo -u flav TEST_PHP_EXECUTABLE=/usr/local/php$PHP_V/bin/php /home/flav/projects/php/php_$PHP_D/run-tests.php \
	-m \
	--show-diff $*

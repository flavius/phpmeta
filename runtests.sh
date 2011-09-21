#!/bin/bash

PHP_V=$FLAV_ACTIVATED_PHP

sudo -u flav TEST_PHP_EXECUTABLE=/usr/local/php$PHP_V/bin/php $FLAV_SRCACTIVE_PHP/run-tests.php \
    -q \
	$*

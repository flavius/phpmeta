#!/bin/bash

sudo -u flav TEST_PHP_EXECUTABLE=/usr/bin/php php /home/flav/php-5.3.6/run-tests.php \
	-m \
	--show-diff $*

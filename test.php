<?php
$src = 'abc<?php 12345';
$src = '<?php 1';
$src = '';
$src = '<?php 	42';
meta_test($src);

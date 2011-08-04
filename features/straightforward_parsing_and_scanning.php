<?php
//the straightforward set of functions accept either a string or
//a file pointer as parameters, we can use both, unless specified
//otherwise
//
//NOTE: the functions' signatures may change


$src = '<?php $foo = 20+22;';
$file_src = fopen('my_src.php', 'r');

//--------- scenario 1 ------------
//we can get the AST tree of a source code directly,

$tree = meta_getast($src);

//--------- scenario 2 ------------
//or we can get the tokens one by one and feed the parser,
//thus having the chance to also filter away or annotate the
//tokens

$scanner = meta_scanner_init($src);
$parser = meta_parser_init();
while ($token = meta_scanner_get($scanner)) {
    //... do some transformations ... for instance we could turn all the
    //T_OPEN_TAGS into "<?php", and all the short variants into two tokens,
    //"<?php" and "echo", then the expression
    meta_parser_feed($parser, $token);
}

$tree = meta_parser_tree($parser);

meta_scanner_destroy($parser);
meta_parser_destroy($scanner);

//--------- scenario 3 ------------
//an xpath-like language is also planned, but only for forward matching, so
//no ".." and the like (as they don't make sense anyway)
//
//here is only a sample of how it MAY look like, the concrete names of the
//nodes and attributes is still a work in progress
//
//TODO abstract semantic graph
//
//these treepaths will be cached for the entire lifetime of the script, as
//the extension will have to generate an AST for each of them too.
$cache_paths = array(
    '/directive[@name="include"]',//all top-level include directives in the script
    'operator[@name="+"]',//all additions
    'operator',//all operators - TODO: compare trees of both treepaths, do optimizations if subtree
    'function[@identifier="foo"]',//the function named "foo"
    'directive[@name="return"]', //all return statemets
    'function[@identifier="bar"]/directive[@name="return"]', //all return statements of the bar() function
    'function[directive[@name="return"]//value]',// all functions which return constant values
);
//like above, but with an additional parameter
//
$parser = meta_parser_init($cache_paths);
//.. idem
$tree = meta_parser_tree($parser);

$node = meta_parser_getnode($cache_paths[0]);

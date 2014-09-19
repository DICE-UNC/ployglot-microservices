get_extensions_test {
 	msiGetExtension("test.txt.gif", *out);
	writeLine('stdout', *out);  
}
input null
output ruleExecOut

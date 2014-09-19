irods_curl_get_test {
	irods_curl_get(*url, *input_obj, "png", *out);
        
	if(*out == ""){
		writeLine('stdout', "ERROR");
	}
}
input *url="http://129.25.12.151/", *input_obj="/tempZone/home/public/new.jpg"
output ruleExecOut

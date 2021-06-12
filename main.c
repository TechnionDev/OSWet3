

#include <stdio.h>
#include "log.h"


int main(){
	debug("Hello");
	debug("Hello %s", "World");
	warning("This is a warning");
	info("This is info");
	error("This is an error");

}

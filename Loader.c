void set_progaddr(){
	int ret;
	char addr[10];

	addr[0] = '\0';
	PROGADDR =  0;
	ret = Get_String_Argument(addr);
	Success = TRUE;
	if( ret != ENTER ){
		Success = FALSE;
		printf("Please enter proper argument\n");
		return;
	}
	if( addr[0] != '\0' ){
		ret = Str_convert_into_Hex(addr, &PROGADDR );
		if( ret == FALSE ){
			printf( "Please enter proper argument\n");
			return;
		}
	}
	printf("Program starting address set to 0x");
	Hex_convert_into_Str( PROGADDR, 4);
	printf("\n");
}

void load(){
}

void run(){

}
void set_bp(){
}

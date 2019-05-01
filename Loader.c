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
	Object_file_info linking_files[3];
	FILE* curfp;
	unsigned int CSADDR;
	int linking_num;
	char filename[MAX_COMMAND];
	char garage[73];
	char tmp[10];
	char ch;
	estab_node *new_node;
	unsigned num;
	int ret, i;

	// set File pointer
	for( i = 0; i < 3 ; i++){

		linking_files[i].symlist = NULL;
		linking_files[i].tail = NULL;

		// read arguments
		ret = Get_String_Argument( filename );
		if( i == 2 && ret != ENTER ){
			printf("Too many argument.\n");
			Success = FALSE;
		}
		linking_files.filepoint = fopen( filename , "r" );
		linking_num++;
		if( ret == ENTER )
			break;
	}
	if( Success == FALSE || linking_num == 0 ){
		Success = FALSE;
		return;
	}

	CSADDR = PROGADDR;
	// Pass1
	for( i = 0 ; i < linking_num ; i++ ){
		curfp = linking_files[i].filepoint;
		ch = fscanf( curfp, "%c", &ch );
		if( ch != 'H' ){
			printf("H record doesn't exist.\n");
			Success = FALSE;
			break;
		}
		// Read H record
		new_node = (estab_node *)malloc( sizeof( estab_node ));
		Read_File( curfp, new_node.name, 6 );
		Read_File( curfp, garage, 6 );
		Read_File( curfp, tmp, 7 );
		Str_convert_into_Hex( tmp, &num );
		linking_files[i].length = num;
		new_node.address = CSADDR;
		new_node.next = NULL;
		Success = push_into_ESTAB( new_node );
		linking_files[i].symlist = linking_files[i].tail = new_node;
		if( Success == FALSE )
			break;


		// Read D record
		fscanf( curfp, "%c", &ch );
		if( ch != 'D' ){
			printf("D record doesn't exist.\n");
			Success = FALSE;
			return;
		}

		while( FALSE ){
			new_node = ( estab_node *)malloc( sizeof( estab_node ));
			ret = Read_File( curfp, new_node.name, 6 );
			if( ret == ENTER ){
				printf("Error in D record.\n");
				Success = FALSE;
				break;
			}
			ret = Read_File( curfp, tmp, 6 );
			Str_convert_into_Hex( tmp, &num );
			num += CSADDR;
			new_node.address = num;
			new_node.next = NULL;
			Success = push_into_ESTAB( new_node );
			if( Success == FALSE )
				break;
			linking_files[i].tail->next = new_node;
			linking_files[i].tail = new_node;
			if( ret == ENTER )
				break;
		}
		if( Success == FALSE )
			break;
		CSADDR += linking_files[i].length;
	}
	if( Success == FALSE ){
		erase_ESTAB();
		return;
	}
	// Pass2


	// File close
	for( i = 0; i< linking_num ; i++ )
		fclose( linking_files.filepoint );

	erase_ESTAB();
}

// -------------------------   Deal with ESTAB --------------------------------
int push_into_ESTAB( estab_node* new_node ){
	// return FALSE if overlap occurs
}

void erase_ESTAB(){
	for( i = 0; i < ESTAB_SIZE ; i++ ){
		free( ESTAB[i] );
		ESTAB[i] = NULL;
	}
}

void run(){

}
void set_bp(){
}

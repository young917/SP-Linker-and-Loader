#include "20171697.h"
#include "variable.h"

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
	estab_node *new_node, *cur;
	unsigned addr, len, val;
	int ret, i, j;

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
		for( i = 0; i< linking_num ; i++ )
			fclose( linking_files.filepoint );
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
		Str_convert_into_Hex( tmp, &len );
		linking_files[i].length = len;
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
			for( i = 0; i< linking_num ; i++ )
				fclose( linking_files.filepoint );
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
			Str_convert_into_Hex( tmp, &addr );
			addr += CSADDR;
			new_node.address = addr;
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
		for( i = 0; i< linking_num ; i++ )
			fclose( linking_files.filepoint );
		return;
	}

	// Pass2
	CSADDR = PROGADDR;
	for( i = 0 ; i < linking_num && Success == TRUE ; i++ ){
		curfp = linking_files[i].filepoint;
		
		while( Success ){
			fscanf(curfp, "%c", &ch);
			if( ch == 'R' ){
				Read_File( curfp, garage, 73 );
			}
			else if( ch == 'T' ){
				Read_File( curfp, tmp, 6 );
				Str_convert_into_Hex( tmp, &addr );
				addr += CSADDR;
				Read_File( curfp, tmp, 2 );
				Str_convert_into_Hex( tmp, &len );
				for( j = 0; j < len; j++, addr++ ){
					ret = Read_File( curfp, tmp, 2 );
					Str_convret_into_Hex( tmp, &num );
					Memory[addr] = num;
					if( ret == ENTER && j != (len - 1) ){
						printf("Error in T record.\n");
						Success == FALSE;
						break;
					}
				}
				ret = Read_File( curfp, tmp, 1 );
			}
			else if( ch == 'M' ){
				Read_File( curfp, tmp, 6 );
				Str_convert_into_Hex( tmp, &addr );
				addr += CSADDR;
				Read_File( curfp, tmp, 2 );
				Str_convert_into_Hex( tmp, &len );
				len = ( len + 1 ) / 2;
				
				num = 0;
				for( j = 0 ; j < len ; j++ ){
					num *= 16*16;
					num += Memory[ addr + j ];
				}
				fscanf( curfp,"%c", &ch);
				Read_File( curfp, tmp, 7 );
				val = find_in_ESTAB( tmp );
				if( ch == '+' ){
					num += val;
					num %= 0x1000000;
				}
				else{
					val = val ^ 0xFFFFFF;
					val += 1;
					num += val;
					num %= 0x1000000;
				}
				for( j = len - 1 ; j >= 0 ; j-- ){
					val = num %  0x100;
					num /= 0x100;
					Memory[ addr + j ] = val;
				}
			}
			else if( ch == 'E' )
				break;
			else{
				printf("Error exists.\n");
				Success = FALSE;
			}
		}
	}
	if( Success == FALSE ){
		erase_ESTAB();
		for( i = 0; i< linking_num ; i++ )
			fclose( linking_files.filepoint );
		return;
	}	

	// Print Load Map
	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("----------------------------------------------------------");
	len = 0;
	for( i = 0; i < listing_num ; i++ ){
		cur = listing_files.symlist;
		printf("%s\t\t      \t\t", cur.name);
		Hex_convert_into_Str( cur.address, 4 );
		printf("\t\t");
		Hex_convert_into_Str( listing.files.length );
		len += listing.files.length;

		cur = cur->next;
		while ( cur != NULL ){
			printf("       \t\t%s\t\t", cur.name);
			Hex_convert_into_Str( cur.address, 4 );
			printf("\n");
			cur = cur->next;
		}
	}
	printf("       \t\t      \t\ttotal length\t");
	Hex_convert_into_Str( len, 4 );
	printf("\n");

	// File close
	for( i = 0; i< linking_num ; i++ )
		fclose( linking_files.filepoint );

	erase_ESTAB();
}

// -------------------------   Deal with ESTAB --------------------------------
int push_into_ESTAB( estab_node* new_node ){
	// return FALSE if overlap occurs
	int hash_key;
	estab_node *cur;

	hash_key = Hash_func( new_node.name, ESTAB_SIZE );
	cur = ESTAB[hash_key];
	while( cur->next != NULL ){
		cur = cur->next;
	}
	cur->next = new_node;
}

unsigned int find_in_ESTAB( char str[] ){
	int hash_key;
	unsigned int ret = 0;
	estab_node *cur;

	hash_key = Hash_func( str, ESTAB_SIZE );
	cur = ESTAB[hash_key];
	while( cur != NULL ){
		if( strcmp( str, cur.name ) == 0 ){
			ret = cur.address;
			break;
		}
	}
	return ret;
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

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
	Object_File_Info linking_files[3];
	FILE* curfp;
	unsigned int CSADDR;
	unsigned int Reference_Table[14];
	int linking_num;
	char filename[MAX_COMMAND];
	char garage[73];
	char tmp[10];
	char ch;
	estab_node *new_node, *cur;
	unsigned addr, len, val, num;
	int ret, i, j;

	// set File pointer
	for( i = 0; i < 3 ; i++){
		linking_files[i].filename[0] = '\0';
		linking_files[i].header = NULL;
		linking_files[i].symlist = NULL;
		linking_files[i].tail = NULL;

		// read arguments
		ret = Get_String_Argument( filename );
		if( i == 2 && ret != ENTER ){
			printf("Too many argument.\n");
			Success = FALSE;
		}
		strcpy ( linking_files[i].filename, filename );
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
		curfp = fopen( linking_files[i].filename, "r" );
		while( Success ){
			fscanf( curfp, "%c", &ch );

			if( ch == 'H' ){
				new_node = (estab_node *)malloc( sizeof( estab_node ));
				Read_File( curfp, new_node->name, 6 );
				Read_File( curfp, garage, 6 );
				Read_File( curfp, tmp, 7 );
				Str_convert_into_Hex( tmp, &len );
				linking_files[i].length = len;
				new_node->address = CSADDR;
				new_node->next = NULL;
				Success = push_into_ESTAB( new_node );
				linking_files[i].header = new_node;
			}

			else if( ch == 'D' ){
				while( TRUE ){
					new_node = ( estab_node *)malloc( sizeof( estab_node ));
					ret = Read_File( curfp, new_node->name, 6 );
					if( ret == ENTER ){
						printf("Error in D record.\n");
						Success = FALSE;
						break;
					}
					ret = Read_File( curfp, tmp, 6 );
					Str_convert_into_Hex( tmp, &addr );
					addr += CSADDR;
					new_node->address = addr;
					new_node->next = NULL;
					Success = push_into_ESTAB( new_node );
					if( Success == FALSE )
						break;
					if( linking_files[i].symlist == NULL ){
						linking_files[i].symlist = new_node;
						linking_files[i].tail = new_node;
					}
					else{
						linking_files[i].tail->next = new_node;
						linking_files[i].tail = new_node;
					}
					if( ret == ENTER )
						break;
				}	
			}

			else if( ch == '.' ){
				Read_File( curfp, garage, 72 );
				continue;
			}
			
			else{
				Read_File( curfp, garage, 72 );
				break;
			}
		}
		fclose( curfp );
		if( Success == FALSE )
			break;

		CSADDR += linking_files[i].length;
	}
	if( Success == FALSE ){
		erase_ESTAB();
		return;
	}

	// Pass2
	CSADDR = PROGADDR;
	for( i = 0 ; i < linking_num && Success == TRUE ; i++ ){
		curfp = fopen( linking_files[i].filename, "r" );
		while( Success ){
			fscanf(curfp, "%c", &ch);

			if( ch == 'R' ){
				// make reference table
				Reference_Table[1] = CSADDR;
				while( TRUE ) {
					ret = Read_File( curfp, tmp, 2 );
					Str_convert_into_Hex( tmp, &num );
					ret = Read_File( curfp, tmp, 6 );
					addr = find_in_ESTAB( tmp );
					Reference_Table[num] = addr;
					if( ret == Eof )
						break;
				}
			}

			else if( ch == 'T' ){
				Read_File( curfp, tmp, 6 );
				Str_convert_into_Hex( tmp, &addr );
				addr += CSADDR;
				Read_File( curfp, tmp, 2 );
				Str_convert_into_Hex( tmp, &len );
				for( j = 0; j < len; j++, addr++ ){
					ret = Read_File( curfp, tmp, 2 );
					Str_convert_into_Hex( tmp, &val );
					Memory[addr] = val;
					if( ret != CHAR && j != (len - 1) ){
						printf("Error in T record.\n");
						Success = FALSE;
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
				if( ch == '\n' ){
					num += Reference_Table[1];
					num %= 0x1000000;
				}
				else{
					Read_File( curfp, tmp, 2 );
					Str_convert_into_Hex( tmp, &num );
					val = Reference_Table[num];

					if( ch == '+' ){
						num += val;
						num %= 0x1000000;
					}
					else if( ch == '-' ){
						val = val ^ 0xFFFFFF;
						val += 1;
						num += val;
						num %= 0x1000000;
					}
					Read_File( curfp, garage, 1 );
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
		fclose( curfp );
		CSADDR += linking_files[i].length;
	}
	if( Success == FALSE ){
		erase_ESTAB();
		return;
	}	

	// Print Load Map
	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("----------------------------------------------------------");
	len = 0;
	for( i = 0; i < linking_num ; i++ ){
		cur = linking_files[i].header;
		printf("%s\t\t      \t\t", cur->name);
		Hex_convert_into_Str( cur->address, 4 );
		printf("\t\t");
		Hex_convert_into_Str( linking_files[i].length , 4);
		len += linking_files[i].length;

		cur = linking_files[i].symlist;
		while ( cur != NULL ){
			printf("       \t\t%s\t\t", cur->name);
			Hex_convert_into_Str( cur->address, 4 );
			printf("\n");
			cur = cur->next;
		}
	}
	printf("       \t\t      \t\ttotal length\t");
	Hex_convert_into_Str( len, 4 );
	printf("\n");

	// File close
	erase_ESTAB();
}

// -------------------------   Deal with ESTAB --------------------------------
int push_into_ESTAB( estab_node* new_node ){
	// return FALSE if overlap occurs
	int hash_key;
	int ret = TRUE;
	estab_node *cur;

	hash_key = Hash_func( new_node->name, ESTAB_SIZE );
	cur = ESTAB[hash_key];
	if( cur == NULL ){
		ESTAB[hash_key] = new_node;
		return ret;
	}
	while( cur->next != NULL ){
		if( strcmp( new_node->name, cur->name ) == 0 ){
			ret = FALSE;
			break;
		}
		cur = cur->next;
	}
	if( ret == FALSE )
		return ret;

	cur->next = new_node;
	return ret;
}

unsigned int find_in_ESTAB( char str[] ){
	int hash_key;
	unsigned int ret = 0;
	estab_node *cur;

	hash_key = Hash_func( str, ESTAB_SIZE );
	cur = ESTAB[hash_key];
	while( cur != NULL ){
		if( strcmp( str, cur->name ) == 0 ){
			ret = cur->address;
			break;
		}
	}
	return ret;
}

void erase_ESTAB(){
	int i;

	for( i = 0; i < ESTAB_SIZE ; i++ ){
		free( ESTAB[i] );
		ESTAB[i] = NULL;
	}
}

void run(){

}
void set_bp(){
}

#include "20171697.h"
#include "variable.h"

void set_progaddr(){
	int ret;
	char addr[10];

	addr[0] = '\0';
	ret = Get_String_Argument(addr);
	if( Success == FALSE || ret != ENTER ){// no argument or more than 1 argument
		Success = FALSE;
		printf("\nPlease enter proper argument.\n");
		return;
	}
	if( strncmp(addr, "0x", 2) == 0 || strncmp(addr, "0X", 2) == 0 )
		strcpy(addr, addr+2 );
	ret = Str_convert_into_Hex(addr, &PROGADDR );// argument doesn't consist of hexadecimal string
	if( ret == FALSE || PROGADDR > MEM_LIMIT ){
		printf( "\nPlease enter proper argument\n");
		return;
	}

	printf("\nProgram starting address set to 0x");
	Hex_convert_into_Str( PROGADDR, 4);
	printf("\n\n");
}

void load(){
	Object_File_Info linking_files[3];
	FILE* curfp;
	unsigned int CSADDR;
	unsigned int Reference_Table[14];
	int linking_num = 0;
	char filename[MAX_COMMAND];
	char garage[73];
	char tmp[10];
	char ch;
	estab_node *new_node, *cur, *after, *ext_cur, *ext_before;
	estab_node * extdef[3];
	unsigned addr, len, val, num, idx;
	int ret, i, j;

	// Store arguments
	for( i = 0; i < 3 ; i++){
		linking_files[i].filename[0] = '\0';
	
		// read arguments
		ret = Get_String_Argument( filename );
		if( Success == FALSE )
			break;
		else if( i == 2 && ret != ENTER ){
			printf("Too many argument.\n");
			Success = FALSE;
			break;
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
		if( curfp == NULL ){
			Success = FALSE;
			break;
		}
		while( Success ){
			ret = fscanf( curfp, "%c", &ch );

			if( ret == EOF )
				break;

			else if( ch == 'H' ){
				new_node = (estab_node *)malloc( sizeof( estab_node ));
				Read_File( curfp, new_node->name, 6 );
				Read_File( curfp, garage, 6 );
				Read_File( curfp, tmp, 7 );
				Str_convert_into_Hex( tmp, &len );
				linking_files[i].length = len;
				new_node->address = CSADDR;
				new_node->next = NULL;
				new_node->prognum = i;
				Success = push_into_ESTAB( new_node );
			}

			else if( ch == 'D' ){
				while( TRUE ){
					new_node = ( estab_node *)malloc( sizeof( estab_node ));
					ret = Read_File( curfp, new_node->name, 6 );
					if( ret != CHAR ){// Revise
						free( new_node );
						new_node = NULL;
						break;
					}
					ret = Read_File( curfp, tmp, 6 );
					Str_convert_into_Hex( tmp, &addr ); 
					addr += CSADDR;
					new_node->address = addr;
					new_node->next = NULL;
					new_node->prognum = i;
					Success = push_into_ESTAB( new_node );
				}	
			}
			else{// Revise
				ret = Read_File( curfp, garage, 73 );
				if( ret == Eof )
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

	mem_reset();
	// Pass2
	CSADDR = PROGADDR;
	for( i = 0 ; i < linking_num && Success == TRUE ; i++ ){
		curfp = fopen( linking_files[i].filename, "r" );
		while( Success ){
			ret = fscanf(curfp, "%c", &ch);
			
			if( ret == EOF )
				break;

			else if( ch == 'R' ){
				// make reference table
				Reference_Table[1] = CSADDR;

				while( TRUE ) {
					ret = fscanf(curfp, "%c", &ch);
					if( ret == EOF || ch == '\n' )
						break;
					else if( ch >= '0' && ch <= '9' ){
						tmp[0] = ch;
						fscanf(curfp, "%c", tmp+1 );
						tmp[2] = '\0';
						Str_convert_into_Hex( tmp, &num );
						ret = Read_File( curfp, tmp, 6 );
						Success = find_in_ESTAB( &addr, tmp );
						Reference_Table[num] = addr;
					}
					else{
						ret = Read_File( curfp, garage, 5 ); 
					}

					if( ret == ENTER || Success == FALSE )
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
					if( addr > MEM_LIMIT ){
						Success = FALSE;
						break;
					}
					Memory[addr] = val;
				}
				ret = Read_File( curfp, tmp, 1 );// read enter
			}

			else if( ch == 'M' ){
				Read_File( curfp, tmp, 6 );
				Str_convert_into_Hex( tmp, &addr );
				addr += CSADDR;

				if( addr > MEM_LIMIT ){
					Success = FALSE;
					break;
				}

				Read_File( curfp, tmp, 2 );
				Str_convert_into_Hex( tmp, &len );
				
				num = 0;
				j = 0;
				if( ( len%2 ) == 1 ){
					num = Memory[addr] % 16;
					Memory[addr] = Memory[addr] - num;
					num *= 16;
					j++;
				}
				
				for( ; j < (len+1)/2 ; j++ ){
					num += Memory[ addr + j ];
					if( j == ( (len+1)/2 -1 ) )
						break;
					num *= 16*16;
				}
				fscanf( curfp,"%c", &ch);
				if( ch == '\n' ){
					num += CSADDR;
				}
				else{
					Read_File( curfp, tmp, 7 );
					if( tmp[0] >= '0' && tmp[0] <= '9' ){// reference number
						Str_convert_into_Hex( tmp, &idx );
						val = Reference_Table[idx];
					}
					else{
						Success = find_in_ESTAB( &val, tmp );
						if( Success == FALSE )
							break;
					}

					if( ch == '+' ){
						num += val;
					}
					else if( ch == '-' ){
						val = val ^ 0xFFFFFF;
						val += 1;
						num += val;
					}
				}

				find_remain( &num, len );

				for( j = (len+1)/2 - 1 ; j > 0 ; j-- ){
					val = num %  0x100;
					num /= 0x100;
					Memory[ addr + j ] = val;
				}
				if( len % 2 == 1 ){
					val = num % 0x10;
					Memory[ addr ] += val;
				}
				else{
					val = num % 0x100;
					Memory[ addr ] = val;
				}
			}

			else if( ch == 'E' ){
				tmp[0] = '\0';
				Read_File( curfp, tmp, 7 );
				if( tmp[0] == '\0' )
					STARTADDR = PROGADDR;
				else{
					Str_convert_into_Hex( tmp, &addr );
					STARTADDR = addr + PROGADDR;
				}
				break;
			}

			else{
				Read_File( curfp, garage, 73 );
			}
		}
		fclose( curfp );
		CSADDR += linking_files[i].length;
	}
	if( Success == FALSE ){
		erase_ESTAB();
		mem_reset();
		return;
	}	

	
	/* Gather external defined variable for each program.
	   And sort them by increasing address */
	for( i = 0; i < linking_num ; i++ )
		extdef[i] = NULL;
	for( i = 0; i < ESTAB_SIZE ; i++ ){
		after = ESTAB[i];
		while( after != NULL ){
			cur = after;
			after = cur->next;
			num = cur->prognum;
			cur->next = NULL;

			ext_before = NULL;
			ext_cur = extdef[num];
			while( ext_cur != NULL ){
				if( ext_cur->address > cur->address )
					break;
				ext_before = ext_cur;
				ext_cur = ext_cur->next;
			}
			if( ext_before == NULL )
				extdef[num] = cur;
			else{
				ext_before->next = cur;
				cur->next = ext_cur;
			}
		}
	}


	// Print Load Map
	printf("\n");
	printf("control\t\tsymbol\t\taddress\t\tlength\n");
	printf("section\t\tname\n");
	printf("----------------------------------------------------------\n");
	len = 0;
	for( i = 0; i < linking_num ; i++ ){
		cur = extdef[i];
		printf("%s\t\t      \t\t", cur->name);
		Hex_convert_into_Str( cur->address, 4 );
		printf("\t\t");
		Hex_convert_into_Str( linking_files[i].length , 4);
		printf("\n");
		len += linking_files[i].length;

		cur = cur->next;
		while ( cur != NULL ){
			printf("       \t\t%s\t\t", cur->name);
			Hex_convert_into_Str( cur->address, 4 );
			printf("\n");
			cur = cur->next;
		}
	}
	printf("----------------------------------------------------------\n");
	printf("       \t\t      \t\ttotal length\t");
	Hex_convert_into_Str( len, 4 );
	printf("\n");
	ENDADDR = PROGADDR + len;
	execution.registers[PC] = STARTADDR;
	for( i = 0; i < 10; i++ ){
		if( i == PC )
			continue;
		else if( i == L )
			execution.registers[L] = ENDADDR;
		else
			execution.registers[i] = 0;
	}


	//Erase extdef array, ESTAB
	for( i = 0; i < linking_num ; i++ ){
		ext_cur = extdef[i];
		while( ext_cur  != NULL ){
			ext_before = ext_cur;
			ext_cur = ext_cur->next;
			free( ext_before );
			ext_before = NULL;
		}
		extdef[i] = NULL;
	}
	for( i = 0 ; i < ESTAB_SIZE ; i++ )
		ESTAB[i] = NULL;
}

void find_remain( unsigned int *val, unsigned int len ){

	unsigned int divisor = 1;
	unsigned int i;

	for( i = 0; i< len ; i++ ){
		divisor = divisor<<4;
	}
	*val = (*val)%divisor;
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

int find_in_ESTAB( unsigned int *addr, char str[] ){
	// return find or not

	int hash_key;
	int ret = FALSE;
	estab_node *cur;

	hash_key = Hash_func( str, ESTAB_SIZE );
	cur = ESTAB[hash_key];
	while( cur != NULL ){
		if( strcmp( str, cur->name ) == 0 ){
			*addr = cur->address;
			ret = TRUE;
			break;
		}
		cur = cur->next;// Revise
	}
	return ret;	
}

void erase_ESTAB(){
	int i;
	estab_node *before, *cur;

	for( i = 0; i < ESTAB_SIZE ; i++ ){
		cur = ESTAB[i];
		while( cur != NULL ){
			before = cur;
			cur = cur->next;
			free( before );
			before = NULL;
		}
		ESTAB[i] = NULL;
	}
}

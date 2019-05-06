#include "20171697.h"
#include "variable.h"

void print_regs(){
		printf("\n");
		printf("A : ");
		Hex_convert_into_Str( execution.registers[A], 6 );
		printf("  X : ");
		Hex_convert_into_Str( execution.registers[X], 6 );
		printf("\n");
		printf("L : ");
		Hex_convert_into_Str( execution.registers[L], 6 );
		printf("  PC: ");
		Hex_convert_into_Str( execution.registers[PC], 6 );
		printf("\n");
		printf("B : ");
		Hex_convert_into_Str( execution.registers[B], 6 );
		printf("  S : ");
		Hex_convert_into_Str( execution.registers[S], 6 );
		printf("\n");
		printf("T : ");
		Hex_convert_into_Str( execution.registers[T], 6 );
		printf("\n");
}
void run(){
	int i;
	unsigned int mem;
	unsigned int tmp;
	BreakPoint *pointer;
	int reg_idx;

	//There has to be no argument
	if( Handling_Input( Erase_space, new_input->str, NULL, 0, FALSE) != ENTER ){
		Success = FALSE;
		return;
	}

	while( TRUE ){
		decode();
		switch( execution.opcode ){
			case LDA: Load( A, 3 ); break;
			case LDB: Load( B, 3 ); break;
			case LDT: Load( T, 3 ); break;
			case LDCH: Load( A, 1 ); break;

			case STA: Store( A, 3 ); break;
			case STX: Store( X, 3 ); break;
			case STL: Store( L, 3 ); break;
			case STCH: Store( A, 1 ); break;
			case J:{
					   execution.registers[PC] = execution.target;
					   break;
				   }
			case JSUB:{
						  execution.registers[L] = execution.registers[PC];
						  execution.registers[PC] = execution.target;
						  break;
					  }
			case JLT:{
						 if( execution.registers[SW] == 0 )
							 execution.registers[PC] = execution.target;
						 break;
					 }
			case JEQ:{
						 if( execution.registers[SW] == 1 )
							 execution.registers[PC] = execution.target;
						 break;
					 }
			case RSUB:{
						  execution.registers[PC] = execution.registers[L];
						  break;
					  }
			case COMP:{
						  if( execution.flag[0] == 0 && execution.flag[1] == 1 ){//immediate address
							  mem = execution.target;
						  }
						  else{
							  mem = 0;
							  for( i = 0 ; i < 3; i++ ){
							  mem = mem << 8;
							  mem += Memory[ execution.target + i ];
						  	  }
						  }

						  tmp = execution.registers[A];
						  if( tmp < mem )
							  execution.registers[SW] = 0;
						  else if( tmp == mem )
							  execution.registers[SW] = 1;
						  else
							  execution.registers[SW] = 2;

						  break;
					  }
			case COMPR:{
						   //Revise
						  reg_idx = execution.target / 16;
						  mem = execution.registers[reg_idx];

						  reg_idx = execution.target % 16;
						  tmp = execution.registers[reg_idx];

						  if( mem < tmp )
							  execution.registers[SW] = 0;
						  else if( mem == tmp )
							  execution.registers[SW] = 1;
						  else
							  execution.registers[SW] = 2;
						  break;
					   }
			case CLEAR:{
						   reg_idx = execution.target / 16;
						   execution.registers[reg_idx] = 0;
						   break;
					   }
			case TIXR:{
						  execution.registers[X] += 1;
						  mem = execution.registers[X];

						  reg_idx = execution.target / 16;
						  tmp = execution.registers[reg_idx];
						  
						  if( mem < tmp )
							  execution.registers[SW] = 0;
						  else if( mem == tmp )
							  execution.registers[SW] = 1;
						  else
							  execution.registers[SW] = 2;

						  break;
					  }
			case TD:{
						execution.registers[SW] = 0;
						break;
					}
			case RD:{
						execution.registers[A] = execution.registers[A] / 0x100;
						break;
				}

		}

		pointer = meet_breakpoint();

		if( pointer != NULL ){

			print_regs();
			printf("Stop at checkpoint[");
			Hex_convert_into_Str( pointer->address , 4);
			printf("]\n\n");

			if( execution.registers[PC] == ENDADDR ){
				printf("End program\n\n");
				execution.registers[PC] = STARTADDR;
				for( i = 0; i < 10; i++ ){
					if( i == PC )
						continue;
					else if( i == L )
						execution.registers[L] = ENDADDR;
					else
						execution.registers[i] = 0;
				}
			}
			break;

		}
		else if( execution.registers[PC] == ENDADDR ){

			print_regs();
			printf("End program\n\n");
			execution.registers[PC] = STARTADDR;
			for( i = 0; i < 10; i++ ){
				if( i == PC )
					continue;
				else if( i == L )
					execution.registers[L] = ENDADDR;
				else
					execution.registers[i] = 0;
			}
			break;
		}

	}
}

BreakPoint * meet_breakpoint(){
	// no exist -> return NULL | exist -> return corresponding BreakPoint*
	BreakPoint *cur;
	unsigned int current_addr;

	current_addr = execution.registers[PC];
	cur = bp_list.list;
	while( cur != NULL ){
		if( current_addr == cur->address )
			break;
		cur = cur->next;
	}
	if( cur == NULL )
		return NULL;
	else
		return cur;
}

void decode(){
	unsigned int pc;
	unsigned int mem;
	unsigned int tmp;
	int i;

	pc = execution.registers[PC];

	// Judge type 2
	tmp = Memory[pc];
	if( tmp == 0xA0 || tmp == 0xB4 || tmp == 0xB8 ){
		execution.opcode = tmp;
		execution.registers[PC] += 2;
		execution.target = Memory[pc+1];
		return;
	}

	// Get opcode
	mem = Memory[pc];
	execution.opcode = mem / 4;
	execution.opcode = execution.opcode << 2;

	// Get nixbpe flags
	mem %= 4;
	execution.flag[0] = mem/2;
	execution.flag[1] = mem%2;
	
	mem = Memory[pc+1];
	tmp = 128;
	for( i = 2 ; i < 6; i++ ){
		execution.flag[i] = mem/tmp;
		mem %= tmp;
		tmp = tmp >> 1;
	}
	execution.target = mem;

	// Increase PC
	if( execution.flag[5] == 1 ){
		execution.registers[PC] += 4;
		mem = mem << 8;
		mem += Memory[pc+2];
		mem = mem << 8;
		mem += Memory[pc+3];
		execution.target = mem;
	}
	else{
		execution.registers[PC] += 3;
		mem = mem << 8;
		mem += Memory[pc+2];
		execution.target = mem;
	}

	// Get target address

	//-------- calculate TA
	if ( execution.flag[3] ){// B flag
		execution.target += execution.registers[B];
	}
	else if( execution.flag[4] ){// P flag
		if( ( execution.target / 0x800 ) == 1 ){
			tmp = execution.target ^ 0xFFF;
			tmp = tmp + 0x1;
			execution.target = execution.registers[PC] - tmp;
		}
		else
			execution.target += execution.registers[PC];
	}
	if( execution.flag[2] )// X flag
		execution.target += execution.registers[X];

	//------- interpret TA
	if( execution.flag[0] == 1 && execution.flag[1] == 0 ){// indirect addr
		tmp = execution.target;
		execution.target = 0;
		for( i = 0; i< 3; i++ ){
			execution.target = execution.target << 8;
			execution.target += Memory[tmp + i];
		}
	}
}

void Load(int reg_idx, int len){
	unsigned int mem;
	unsigned int ta;
	int i;

	if( execution.flag[0] == 0 && execution.flag[1] == 1 )//immediate address
		execution.registers[reg_idx] = execution.target;
	else{
		mem = 0;
		ta = execution.target;

		for( i = 0; i< len ; i++ ){
			mem = mem << 8;
			mem += Memory[ta+i];
		}
		// Revise
		if( len == 1 ){
			execution.registers[reg_idx] /= 0x100;
			execution.registers[reg_idx] += mem;
		}
		else
			execution.registers[reg_idx] = mem ;

	}
}

void Store(int reg_idx, int len){
	unsigned int ta;
	unsigned int val;
	int i;

	ta = execution.target;
	val = execution.registers[reg_idx];
	for( i = len - 1; i >= 0 ; i-- ){
		Memory[ ta+ i ] = val % 0x100;
		val /= 0x100;
	}
	
}

void set_bp(){
	char arg[10];
	unsigned int addr;
	int ret;

	arg[0] = '\0';
	ret = Get_String_Argument( arg );
	Success = TRUE;
	if( ret != ENTER ){// more than one argument
		Success = FALSE;
		return;
	}
	else if( arg[0] == '\0' ){// no argument : show all break points
		show_bplist();
	}
	else{
		if( strcmp( arg, "clear" ) == 0 ){// breakpoint clear
			delete_bplist();
			printf("\n[ok] clear all breakpoints\n");
			return;
		}
		// Revise
		if( strncmp( arg, "0x", 2 ) == 0 || strncmp( arg, "0X", 2 ) == 0 )
			strcpy( arg, arg+2 );
		ret = Str_convert_into_Hex( arg, &addr );
		if( ret == FALSE ){
			Success = FALSE;
			return;
		}
		ret = push_into_bplist( addr );
		if( ret == TRUE )
				printf("\n[ok] create breakpoint %s\n", arg);
	}

}
void delete_bplist(){
	BreakPoint *cur, *before;
	
	cur = bp_list.list;
	while( cur != NULL ){
		before = cur;
		cur = cur->next;
		free( before );
		before = NULL;
	}
	bp_list.list = NULL;
	bp_list.tail = NULL;
}

void show_bplist(){
	BreakPoint *cur;

	cur = bp_list.list;
	if( cur == NULL ){
		printf("\nno breakpoints set.\n");
		return;
	}

	printf("\nbreakpoints\n------------------\n");
	while( cur != NULL ){
		Hex_convert_into_Str( cur->address, 4 );
		printf("\n");
		cur = cur->next;
	}
}

int push_into_bplist(unsigned int addr){
	// return FALSE: already exist  TRUE: successfully push into bplist
	BreakPoint *cur;
	BreakPoint *new_node;

	// Judge is this bp already set 
	cur = bp_list.list;
	while( cur != NULL ){
		if( cur->address == addr )
			break;
		cur = cur->next;
	}
	if( cur != NULL )
		return FALSE;

	// push into bplist
	new_node = (BreakPoint *)malloc( sizeof( BreakPoint ));
	new_node->address = addr;
	new_node->next = NULL;

	if( bp_list.list == NULL ){
		bp_list.list = new_node;
		bp_list.tail = new_node;
	}
	else{
		bp_list.tail->next = new_node;
		bp_list.tail = new_node;
	}
	return TRUE;
}

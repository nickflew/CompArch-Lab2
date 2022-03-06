/***************************************************************/
/*                                                             */
/*   ARMv4-32 Instruction Level Simulator                      */
/*                                                             */
/*   ECEN 4243                                                 */
/*   Oklahoma State University                                 */
/*                                                             */
/***************************************************************/

#ifndef _SIM_ISA_H_
#define _SIM_ISA_H_
#define N_CUR ( (CURRENT_STATE.CPSR>>31) & 0x00000001 )
#define Z_CUR ( (CURRENT_STATE.CPSR>>30) & 0x00000001 )
#define C_CUR ( (CURRENT_STATE.CPSR>>29) & 0x00000001 )
#define V_CUR ( (CURRENT_STATE.CPSR>>28) & 0x00000001 )
#define N_NXT ( (NEXT_STATE.CPSR>>31) & 0x00000001 )
#define Z_NXT ( (NEXT_STATE.CPSR>>30) & 0x00000001 )
#define C_NXT ( (NEXT_STATE.CPSR>>29) & 0x00000001 )
#define V_NXT ( (NEXT_STATE.CPSR>>28) & 0x00000001 )

#define N_N 0x80000000
#define Z_N 0x40000000
#define C_N 0x20000000
#define V_N 0x10000000

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"


//ArithMetic Shift
int ars(int left, int right) {
  int result;
  if(left < 0) result = (left >> right) &  0x80000000;
  else result = (left >> right);
  return result; 
}

//overflow test
int ovflow(int a, int b, int c) {  //if a and b are positive and c is negative overlow
  int truth;                       //if a and b are negative and c is positive overflow
  if( ((a > 0) & (b > 0) & (c < 0)) | ((a < 0) & (b < 0) & (c > 0) ) ) truth = 1;
  else truth = 0;
  return truth;
}

//rotate shift
int rotate(int left, int right) {
  int result;
  result = (left >> right) | (left << (32 - right));
  return result;
}


//ADD Function
int ADD (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;

}





//Add With Carry Function
int ADC (int Rd, int Rn, int Operand2, int I, int S, int CC) {
int cur = 0;
int overflow;
if(I == 0) {
  int sh = (Operand2 & 0x00000060) >> 5;
  int shamt5 = (Operand2 & 0x00000F80) >> 7;
  int bit4 = (Operand2 & 0x00000010) >> 4;
  int Rm = Operand2 & 0x0000000F;
  int Rs = (Operand2 & 0x00000F00) >> 8;

  if(bit4 == 0) {
    switch(sh) {

      //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
      (CURRENT_STATE.REGS[Rm] << shamt5);
      overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
      break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] +
      (CURRENT_STATE.REGS[Rm] >> shamt5);
      overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
      break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] +
      (CURRENT_STATE.REGS[Rm] >> shamt5) |
      (CURRENT_STATE.REGS[Rm] << (32-shamt5));
      overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);
      break;
    }

  } else {
    switch (sh) {

      //logical shift left reg
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //logical shift right reg
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right reg
      case 2: { 
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur); 
        break;
      }

      //rotate shift right reg
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
               overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
	  break;
      }

  } 

}

  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
     overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }

  NEXT_STATE.REGS[Rd] = cur + C_CUR;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if ( overflow == 1)
      NEXT_STATE.CPSR |= V_N;
  }	
  return 0;

}




//AND FUNCTION
int AND (int Rd, int Rn, int Operand2, int I, int S, int CC) {
int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ars(CURRENT_STATE.REGS[Rm], shamt5); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;



}




//ASR Function
int ASR (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int cur = 0;
  
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    
    if( bit4 == 0) cur = ars( CURRENT_STATE.REGS[Rm], shamt5);
    else cur = ars( CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]);

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;




}



//Branch Operation
int B (int imm24, int CC) {
  NEXT_STATE.PC = (CURRENT_STATE.PC + 8) + (imm24 << 2);
  return 0;
}




//Bitwise Clear Operation
int BIC (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
	   ~(CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  ~(CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ~(ars(CURRENT_STATE.REGS[Rm], shamt5)); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ~((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
	  ~(CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  ~(CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ~(ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs])); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ~((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & ~(Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;




}




//Branch with Link Operation
int BL (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  NEXT_STATE.REGS[14] = CURRENT_STATE.PC + 4;
  NEXT_STATE.PC = (CURRENT_STATE.PC + 8) + (imm24 << 2);
  return 0;
}




//Compare Negative Operation
int CMN (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] + 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] + ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] + 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] + (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;


}




//Compare Operation
int CMP (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;

}





//Exclusive OR Operation
int EOR (int Rd, int Rn, int Operand2, int I, int S, int CC){
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] ^ ars(CURRENT_STATE.REGS[Rm], shamt5); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] ^ ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;



}




//Load Register Operation
int LDR (char* i_);





//Load Register with Byte
int LDRB (char* i_);





//Logical Shift Left Operation ASK CALE
int LSL(int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int cur = 0;
  
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    
    if( bit4 == 0) cur = CURRENT_STATE.REGS[Rm] << shamt5;
    else cur = CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs];

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;



  
}





//Logical Shift Right Operation
int LSR (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int cur = 0;
  
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    
    if( bit4 == 0) cur = CURRENT_STATE.REGS[Rm] >> shamt5;
    else cur = CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs];

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;





}





//Multiple Accumulate Operation
int MLA (int Rd, int Rn, int Rm, int Ra, int S, int CC) {

  int cur = CURRENT_STATE.REGS[Rn] * CURRENT_STATE.REGS[Rm];
  cur = cur + Ra;

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;

}





//Move Operation
int MOV (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int cur = 0;
  
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    
  

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;


}






//Multiply Operation
int MUL (int Rd, int Rn, int Rm, int S, int CC) {

  int cur = CURRENT_STATE.REGS[Rn] * CURRENT_STATE.REGS[Rm];

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;

}





//Bitwise NOT Operation
int MVN (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int cur = ~(CURRENT_STATE.REGS[Rn]);

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;

}






//ORR Operation
int ORR(int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] | 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] | 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] | ars(CURRENT_STATE.REGS[Rm], shamt5); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] | 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] | 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] | 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] | ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] | 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] | (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;



}





//Rotate Shift Right Operation
int ROR (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int cur = 0;
  
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    
    if( bit4 == 0) cur = rotate(CURRENT_STATE.REGS[Rm], shamt5);
    else cur = rotate(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]);

  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;



}




//Subtract With Carry Operation
int SBC (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  NEXT_STATE.REGS[Rd] = cur - C_CUR;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;

}




//Store Register
int STR (int Rd, int Rn, int Operand2, int I, int PW, int U) {

}




//Store Byte Register
int STRB (char* i_);





//Subtract Operation
int SUB (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], shamt5);
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] - 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] - ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] - 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] - (Imm>>2*rotate|(Imm<<(32-2*rotate)));
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;

}

//Reverse Subtraction
int RSB (int Rd, int Rn, int Operand2, int I, int S, int CC) {
   int overflow = 0;
  int cur = 0;
  if(I == 0) {
    
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = (CURRENT_STATE.REGS[Rm] << shamt5) - CURRENT_STATE.REGS[Rn];
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << shamt5), cur);
	  break;

      //logical shift right
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> shamt5) - CURRENT_STATE.REGS[Rn];
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> shamt5), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = ars(CURRENT_STATE.REGS[Rm], shamt5) - CURRENT_STATE.REGS[Rn];
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }

      //rotate shift right
      case 3: cur =  ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))) - CURRENT_STATE.REGS[Rn];
           overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5))), cur);    
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]) - CURRENT_STATE.REGS[Rn];
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]), cur);
	  break;

        //logical shift right
      case 1: cur = (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) - CURRENT_STATE.REGS[Rn];
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]), cur);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]) - CURRENT_STATE.REGS[Rn]; 
        overflow = ovflow(CURRENT_STATE.REGS[Rn], (ars(CURRENT_STATE.REGS[Rm], shamt5)), cur);
        break;
      }
	  
      //rotate shift right
      case 3: cur =  ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))) - CURRENT_STATE.REGS[Rn];
              overflow = ovflow(CURRENT_STATE.REGS[Rn], ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs]))), cur);
          
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = (Imm>>2*rotate|(Imm<<(32-2*rotate))) - CURRENT_STATE.REGS[Rn];
    overflow = ovflow(CURRENT_STATE.REGS[Rn], (Imm>>2*rotate|(Imm<<(32-2*rotate))), cur);
  }


  //set conditional flags
  NEXT_STATE.REGS[Rd] = cur;
  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
    if (overflow == 1)
      NEXT_STATE.CPSR |= V_N;
    
    
  }	
  return 0;

}


//Test Equivalence Operation
int TEQ (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] ^ ars(CURRENT_STATE.REGS[Rm], shamt5); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] ^ 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] ^ ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] ^ 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] ^ (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;




}





//Test Operation
int TST (int Rd, int Rn, int Operand2, int I, int S, int CC) {
  int cur = 0;
  if(I == 0) {
    int sh = (Operand2 & 0x00000060) >> 5;
    int shamt5 = (Operand2 & 0x00000F80) >> 7;
    int bit4 = (Operand2 & 0x00000010) >> 4;
    int Rm = Operand2 & 0x0000000F;
    int Rs = (Operand2 & 0x00000F00) >> 8;
    if (bit4 == 0) 
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] << shamt5);
	  break;

      //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] >> shamt5);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ars(CURRENT_STATE.REGS[Rm], shamt5); 
        break;
      }

      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ((CURRENT_STATE.REGS[Rm] >> shamt5) |
               (CURRENT_STATE.REGS[Rm] << (32 - shamt5)));
	  break;
      }     
    else
      switch (sh) {

        //logical shift left
      case 0: cur = CURRENT_STATE.REGS[Rn] &
	  (CURRENT_STATE.REGS[Rm] << CURRENT_STATE.REGS[Rs]);
	  break;

        //logical shift right
      case 1: cur = CURRENT_STATE.REGS[Rn] & 
	  (CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]);
	  break;

      //arithmetic shift right NEEDS CHECKING
      case 2: {
        cur = CURRENT_STATE.REGS[Rn] & ars(CURRENT_STATE.REGS[Rm], CURRENT_STATE.REGS[Rs]); 
        break;
      }
	  
      //rotate shift right
      case 3: cur = CURRENT_STATE.REGS[Rn] & 
	      ((CURRENT_STATE.REGS[Rm] >> CURRENT_STATE.REGS[Rs]) |
               (CURRENT_STATE.REGS[Rm] << (32 - CURRENT_STATE.REGS[Rs])));
	  break;
      }      
  }
  if (I == 1) {
    int rotate = Operand2 >> 8;
    int Imm = Operand2 & 0x000000FF;
    cur = CURRENT_STATE.REGS[Rn] & (Imm>>2*rotate|(Imm<<(32-2*rotate)));
  }

  if (S == 1) {
    if (cur < 0)
      NEXT_STATE.CPSR |= N_N;
    if (cur == 0)
      NEXT_STATE.CPSR |= Z_N;
  }	
  return 0;




}






//Software Interrupt
int SWI (char* i_){return 0;}

#endif

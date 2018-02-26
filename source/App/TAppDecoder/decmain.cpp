/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2015, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWAREs IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     decmain.cpp
    \brief    Decoder application main
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "TAppDecTop.h"

//! \ingroup TAppDecoder
//! \{

#if PIP
int spQ = 7; // 7	16	26	44
int SP_QOFFSET;
//int SP_QOFFSET = 1;
bool multipred = false;
Bool LGB4x8 = true, LGB8x4 = true, LGB8x8 = true, LGB4x4 = true;
TCoeff TMPSpR1[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
Bool SpatialCodeCoeffNxN = false;
Bool firstFrate = true;

int m_cCUQtCbfSCModelPIP_counter[5]		= { 0, 0, 0, 0, 0 };
int m_cCUPIPflag_counter[5]				= { 0, 0, 0, 0, 0 };
int m_cCUR1SpGr_counter[16]				= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int m_cCUR1SpSign_counter				= 0;

int m_cCUQtCbfSCModelPIP_ones[5]		= { 0, 0, 0, 0, 0 };
int m_cCUPIPflag_ones[5]				= { 0, 0, 0, 0, 0 };
int m_cCUR1SpGr_ones[16]				= { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int m_cCUR1SpSign_ones					= 0;

int offset4x4 = 0;
int offset4x8 = 0;
int offset8x4 = 0;
int offset8x8 = 0;

int m_iQP = 22;
Bool depth10 = true;




#if CBF2x2
int cbfMap4x4[16] = {
	0, 0, 1, 1,
	0, 0, 1, 1,
	2, 2, 3, 3,
	2, 2, 3, 3 };

int cbfMap4x8[32] = { 
	0, 0, 1, 1, 2, 2, 3, 3,
	0, 0, 1, 1, 2, 2, 3, 3,
	4, 4, 5, 5, 6, 6, 7, 7,
	4, 4, 5, 5, 6, 6, 7, 7 };

int cbfMap8x4[32] = { 
	0, 0, 1, 1,
	0, 0, 1, 1,
	2, 2, 3, 3,
	2, 2, 3, 3,
	4, 4, 5, 5,
	4, 4, 5, 5,
	6, 6, 7, 7,
	6, 6, 7, 7 };

int cbfMap8x8[64] = { 
	0,	0,	1,	1,	2,	2,	3,	3,
	0,	0,	1,	1,	2,	2,	3,	3,
	4,	4,	5,	5,	6,	6,	7,	7,
	4,	4,	5,	5,	6,	6,	7,	7,
	8,	8,	9,	9,	10, 10, 11, 11,
	8,	8,	9,	9,	10, 10, 11, 11,
	12, 12, 13, 13, 14, 14, 15, 15,
	12, 12, 13, 13, 14, 14, 15, 15 };

int cbf2x2BlMap4x4[4*4] = {
	0, 1, 4, 5 ,
	2, 3, 6, 7 ,
	8, 9, 12, 13 ,
	10, 11, 14, 15 };

int cbf2x2BlMap4x8[8*4] = { 
	0, 1, 8, 9 ,
	2, 3, 10, 11 ,
	4, 5, 12, 13 ,
	6, 7, 14, 15 ,
	16, 17, 24, 25 ,
	18, 19, 26, 27 ,
	20, 21, 28, 29 ,
	22, 23, 30, 31 };
int cbf2x2BlMap8x4[8*4] = { 
	0, 1, 4, 5 ,
	2, 3, 6, 7 ,
	8, 9, 12, 13 ,
	10, 11, 14, 15 ,
	16, 17, 20, 21 ,
	18, 19, 22, 23 ,
	24, 25, 28, 29 ,
	26, 27, 30, 31 };
int cbf2x2BlMap8x8[16*4] = { 
	0, 1, 8, 9,
	2, 3, 10, 11 ,
	4, 5, 12, 13 ,
	6, 7, 14, 15 ,
	16, 17, 24, 25 ,
	18, 19, 26, 27 ,
	20, 21, 28, 29 ,
	22, 23, 30, 31 ,
	32, 33, 40, 41 ,
	34, 35, 42, 43 ,
	36, 37, 44, 45 ,
	38, 39, 46, 47 ,
	48, 49, 56, 57 ,
	50, 51, 58, 59 ,
	52, 53, 60, 61 ,
	54, 55, 62, 63 };
#endif

#endif

// ====================================================================================================================
// Main function
// ====================================================================================================================
int main(int argc, char* argv[])
{


#if PIP

	for (int v = 0; v < argc; v++)
		if (!strcmp(argv[v], "_QP"))
		{
			m_iQP = atoi(argv[v + 1]);
			cout << argv[v] << "\t" << m_iQP << endl;
		}
	FILE* fout = fopen("log.txt", "w");
	fclose(fout);

	if (m_iQP == 22)
	{
		spQ = 7 + SPQ_DEALTA;
		
		if (CUMAX == 8)
		{
			offset4x4 = 2;
			offset4x8 = 1;
			offset8x4 = 2;
			offset8x8 = 1;
		}
		SP_QOFFSET = 2;
	}

	if (m_iQP == 27)
	{
		spQ = 15 + SPQ_DEALTA;
		
		if (CUMAX == 8)
		{
			offset4x4 = -1;
			offset4x8 = -1;
			offset8x4 = -2;
			offset8x8 = -2;
		}
		SP_QOFFSET = 5;
	}

	if (m_iQP == 32)
	{
		spQ = 27 + SPQ_DEALTA;
		
		if (CUMAX == 8)
		{			
			offset4x4 = 2;
			offset4x8 = 1;
			offset8x4 = -2;
			offset8x8 = 2;
		}
		SP_QOFFSET = 8;
	}

	if (m_iQP == 37)
	{
		spQ = 45 + SPQ_DEALTA;
		
		if (CUMAX == 8)
		{
			offset4x4 = 1;
			offset4x8 = 2;
			offset8x4 = 2;
			offset8x8 = 1;
		}
		SP_QOFFSET = 10;
	}

	if (depth10)
		spQ *= 4;

#endif


	// =======================
  Int returnCode = EXIT_SUCCESS;
  TAppDecTop  cTAppDecTop;

  // print information
  fprintf( stdout, "\n" );
  fprintf( stdout, "HM software: Decoder Version [%s] (including RExt)", NV_VERSION );
  fprintf( stdout, NVM_ONOS );
  fprintf( stdout, NVM_COMPILEDBY );
  fprintf( stdout, NVM_BITS );
  fprintf( stdout, "\n" );

  // create application decoder class
  cTAppDecTop.create();

  // parse configuration
  if(!cTAppDecTop.parseCfg( argc, argv ))
  {
    cTAppDecTop.destroy();
    returnCode = EXIT_FAILURE;
    return returnCode;
  }

  // starting time
  Double dResult;
  clock_t lBefore = clock();

  // call decoding function
  cTAppDecTop.decode();

  if (cTAppDecTop.getNumberOfChecksumErrorsDetected() != 0)
  {
    printf("\n\n***ERROR*** A decoding mismatch occured: signalled md5sum does not match\n");
    returnCode = EXIT_FAILURE;
  }

  // ending time
  dResult = (Double)(clock()-lBefore) / CLOCKS_PER_SEC;
  printf("\n Total Time: %12.3f sec.\n", dResult);

  // destroy application decoder class
  cTAppDecTop.destroy();

  return returnCode;
}

//! \}


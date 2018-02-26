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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

/** \file     encmain.cpp
    \brief    Encoder application main
*/

#include <time.h>
#include <iostream>
#include "TAppEncTop.h"
#include "TAppCommon/program_options_lite.h" 

//! \ingroup TAppEncoder
//! \{

#include "../Lib/TLibCommon/Debug.h"

#if PIP
#include <algorithm>
#include "TLibCommon\TComDataCU.h"
Int  AllCUCount = 0;
Int	 PIPValudCUCount = 0;
Int	 PIPCUcount = 0;
Int CBiter;
Bool CBloopEnable = false;
Bool LGB4x8 = true, LGB8x4 = true, LGB8x8 = true, LGB4x4 = true;
int spQ = 20;
int spQ22 = 8;
int spQ27 = 16;
int spQ32 = 16;
int spQ37 = 32;
int SP_QOFFSET;
bool multipred = false;

// =================
int offset4x4 = 0;
int offset4x8 = 0;
int offset8x4 = 0;
int offset8x8 = 0;
// =================


Bool SpatialCodeCoeffNxN = false;
Bool SpatialCodeCoeffNxN_2 = false;
TCoeff TMPSpR1[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
Int cbfStats[4] = { 0, 0, 0, 0 };


Double	totalPIPbits = 0;
Double	totalAllbits = 0;

int							deltaSQP;		// -6, -3, +3, +6
int							combIdx = 23;	// between 0 and 23 for 24 different combination


#if SP_QCHANGE
int	spQisChanged_stat = 0, spQchStat = 0, spQall = 0;
#endif

#if R1_BINARY

int binarization[16][10] = {
	{ 2, 2, 2, 2, 2, 2, 2, 2, 2, 0 }, // 0
	{ 2, 2, 2, 2, 2, 2, 2, 2, 1, 0 }, // 1
	{ 2, 2, 2, 2, 2, 2, 2, 1, 1, 0 }, // 2
	{ 2, 2, 2, 2, 2, 2, 1, 1, 1, 0 }, // 3

	{ 2, 2, 2, 2, 1, 1, 1, 1, 0, 0 }, // 4
	{ 2, 2, 2, 2, 1, 1, 1, 1, 0, 1 }, // 5

	{ 2, 2, 2, 1, 1, 1, 1, 1, 0, 0 }, // 6
	{ 2, 2, 2, 1, 1, 1, 1, 1, 0, 1 }, // 7

	{ 2, 1, 1, 1, 1, 1, 1, 0, 0, 0 }, // 8
	{ 2, 1, 1, 1, 1, 1, 1, 0, 0, 1 }, // 9
	{ 2, 1, 1, 1, 1, 1, 1, 0, 1, 0 }, // 10
	{ 2, 1, 1, 1, 1, 1, 1, 0, 1, 1 }, // 11

	{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 0 }, // 12
	{ 1, 1, 1, 1, 1, 1, 1, 0, 0, 1 }, // 13
	{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 0 }, // 14
	{ 1, 1, 1, 1, 1, 1, 1, 0, 1, 1 }, // 12
};
#endif

#if PVQ
double**						Buff_PVQ_N;
#endif

#if SP_QCHANGE && SP_CHEAT
double cheat_rates[65][4] = 
{
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 9.484391359 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 7.922549008, 12.29174628 },
	{ 14.01192607, 13.66511381, 10.50751151, 4.034358438 },
	{ 14.01192607, 13.66511381, 10.72990393, 12.29174628 },
	{ 14.01192607, 13.66511381, 6.905475495, 12.29174628 },
	{ 14.01192607, 13.66511381, 7.888601676, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 4.452542493 },
	{ 14.01192607, 11.34318572, 2.910789295, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 4.516959221 },
	{ 14.01192607, 4.307561806, 4.177875319, 12.29174628 },
	{ 3.202961891, 10.20568219, 13.31486643, 6.409103231 },
	{ 3.011221797, 1.827091752, 0.817014594, 12.29174628 },
	{ 4.492289813, 4.996228826, 11.72990393, 9.484391359 },
	{ 1.870457511, 1.656335062, 8.066938918, 8.291746281 },
	{ 2.016865599, 5.844934848, 8.855434812, 8.969818186 },
	{ 3.252870127, 3.744760955, 5.72990393, 12.29174628 },
	{ 4.247054476, 3.945724989, 9.314866431, 12.29174628 },
	{ 5.332445967, 3.642745997, 13.31486643, 12.29174628 },
	{ 6.304566934, 7.883754097, 2.939827, 0.544392451 },
	{ 8.129283017, 5.02848919, 13.31486643, 12.29174628 },
	{ 9.552494448, 4.989156777, 13.31486643, 12.29174628 },
	{ 10.68999797, 11.08015131, 8.559978929, 12.29174628 },
	{ 10.20457114, 7.141551854, 13.31486643, 12.29174628 },
	{ 10.84200106, 9.495188809, 13.31486643, 12.29174628 },
	{ 11.20457114, 9.141551854, 4.069313725, 12.29174628 },
	{ 14.01192607, 12.66511381, 13.31486643, 7.647890091 },
	{ 14.01192607, 9.141551854, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 10.99293834, 12.29174628 },
	{ 14.01192607, 11.34318572, 13.31486643, 12.29174628 },
	{ 14.01192607, 10.49518881, 8.671010241, 12.29174628 },
	{ 14.01192607, 12.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 9.22740359, 3.010975511 },
	{ 14.01192607, 12.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 12.08015131, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 9.14494143, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 12.31486643, 6.101921722 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 13.31486643, 12.29174628 },
	{ 14.01192607, 13.66511381, 11.72990393, 12.29174628 }
};
#endif


#if USE_24CLUSTER_PREDICTOR
unsigned clusterOrder[clusterCnt][4] =
{
	{ 3, 2, 1, 0 },
	{ 3, 2, 0, 1 },
	{ 3, 1, 2, 0 },
	{ 3, 1, 0, 2 },
	{ 3, 0, 1, 2 },
	{ 3, 0, 2, 1 },
	{ 2, 3, 1, 0 },
	{ 2, 3, 0, 1 },
	{ 2, 1, 3, 0 },
	{ 2, 1, 0, 3 },
	{ 2, 0, 1, 3 },
	{ 2, 0, 3, 1 },
	{ 1, 2, 3, 0 },
	{ 1, 2, 0, 3 },
	{ 1, 3, 2, 0 },
	{ 1, 3, 0, 2 },
	{ 1, 0, 3, 2 },
	{ 1, 0, 2, 3 },
	{ 0, 2, 1, 3 },
	{ 0, 2, 3, 1 },
	{ 0, 1, 2, 3 },
	{ 0, 1, 3, 2 },
	{ 0, 3, 1, 2 },
	{ 0, 3, 2, 1 }
};

int clusterPredictor[clusterCnt][4] =
{
	{ 1, -1, 1, 0 },
	{ 0, 0, 1, 0 },
	{ 1, 0, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 1, -1, 1, 0 },
	{ 1, 0, 0, 1 },
	{ 1, -1, 1, 0 },
	{ 0, 0, 1, 0 },
	{ 1, -1, 1, 0 },
	{ 1, -1, 1, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 1, 0 },
	{ 1, 0, 0, 0 },
	{ 1, -1, 1, 0 },
	{ 1, -1, 0, 1 },
	{ 0, -1, 1, 1 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 1, 0 },
	{ 1, 0, 0, 0 },
	{ 1, 0, 0, 0 },
	{ 1, -1, 1, 0 },
	{ 1, -1, 1, 0 },
	{ 1, -1, 1, 0 },
	{ 1, 0, 0, 0 }
};
#endif

#if BITPLANE_R1_CODING
UInt	lookup_table[TOTALNUMOFSITUATIONS];
#endif

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

Bool tmpFlag = false;
UInt							Bits1=0, Bits2=0;
Bool							maxExceeded = false;
UInt								sigCnt = 0;
UInt							EMTlog = 0;
#endif

// ====================================================================================================================
// Main function
// ====================================================================================================================

int main(int argc, char* argv[])
 {	
#if PIP

#if BITPLANE_R1_CODING	
	// ifstream ftable("..\\..\\..\\LUT_45927SituationsTo8Contexts.txt", ios::in);
	ifstream ftable("..\\..\\..\\LUT_45927SituationsTo16Contexts.txt", ios::in);
	if (ftable.is_open())
	{		
		cout << TOTALNUMOFSITUATIONS << endl;
		string line, token, delimiter = " ";
		int sitCnt = 0, l, p;
		getline(ftable, line);
		while (sitCnt < TOTALNUMOFSITUATIONS)
		{			
			token = line.substr(0, line.find(delimiter));
			l = token.length();
			line.erase(0, line.find(delimiter) + 1);
			p = atoi(token.c_str());
			lookup_table[sitCnt++] = p;
			if (sitCnt % 1000 == 0)
				cout << sitCnt << endl;
		}

		fprintf(stdout, "look-up table was successfully opened\n");
		ftable.close();
	}
	else
	{
		fprintf(stdout, "failed to open the look-up table\b");
		assert(0);
	}
	// memset(lookup_table, 0, TOTALNUMOFSITUATIONS*sizeof(UInt));
#endif

	FILE* fout = fopen("log.txt", "w");
	fclose(fout);
	fout = fopen("log2.txt", "w");
	fclose(fout);
#if PVQ
	Buff_PVQ_N = (double**)xMalloc(double*, PVQ_MAX_BLOCK*PVQ_MAX_BLOCK*sizeof(double*));
	for (int i = 0; i < PVQ_MAX_BLOCK*PVQ_MAX_BLOCK; i++)
	{
		Buff_PVQ_N[i] = (double*)xMalloc(double, PVQ_MAX_K*sizeof(double));
		memset(Buff_PVQ_N[i], 0, PVQ_MAX_K*sizeof(double));
		for (int k = 0; k < PVQ_MAX_K; k++)
			Buff_PVQ_N[i][k] = TComDataCU::getVecNum(i+1, k+1);
	}

#endif

	// FILE *spatialQn = fopen("SpatialQnt.txt", "w");
	// fclose(spatialQn);
	
	int CBiterCnt = 1;
	for (CBiter = 0; CBiter < CBiterCnt; CBiter++)
	{
#endif
		TAppEncTop  cTAppEncTop;

		// print information
		fprintf(stdout, "\n");
		fprintf(stdout, "HM software: Encoder Version [%s] (including RExt)", NV_VERSION);
		fprintf(stdout, NVM_ONOS);
		fprintf(stdout, NVM_COMPILEDBY);
		fprintf(stdout, NVM_BITS);
		fprintf(stdout, "\n\n");

		// create application encoder class
		cTAppEncTop.create();

		// parse configuration
		try
		{
			if (!cTAppEncTop.parseCfg(argc, argv))
			{
				cTAppEncTop.destroy();
#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
				EnvVar::printEnvVar();
#endif
				return 1;
			}
		}
		catch (df::program_options_lite::ParseFailure &e)
		{
			std::cerr << "Error parsing option \"" << e.arg << "\" with argument \"" << e.val << "\"." << std::endl;
			return 1;
		}

#if PRINT_MACRO_VALUES
		printMacroSettings();
#endif

#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
		EnvVar::printEnvVarInUse();
#endif

		// starting time
		Double dResult;
		clock_t lBefore = clock();

		// call encoding function
		cTAppEncTop.encode();

		// ending time
		dResult = (Double)(clock() - lBefore) / CLOCKS_PER_SEC;
		printf("\n Total Time: %12.3f sec.\n", dResult);

#if PIP	
		if (txtWrite)
		{
			Char curOutAddr[512], buf[20];
			FILE* outCB;
			int base = log2((int)CUMAX) - 1, w, h, i, j, r, v, index, emptyCnt;
			cout << endl << "base:" << base << endl;
			for (j = 0; j < base; j++)
				for (i = 0; i < base; i++)
				{
					index = base*j + i;
					w = 1 << (j + 2); h = 1 << (i + 2);
					strcpy(curOutAddr, OutAddr);
					strcat(curOutAddr, _itoa(w, buf, 10)); strcat(curOutAddr, "x"); strcat(curOutAddr, _itoa(h, buf, 10));
					strcat(curOutAddr, "_"); strcat(curOutAddr, InputFileName.c_str()); strcat(curOutAddr, ".txt");

					outCB = fopen(curOutAddr, "w");
					if (outCB)
						cout << endl << "Write: Successfully opened the codebook at: " << curOutAddr << endl;
					else
						cout << endl << "Write: Error opening the output codebook file at: " << curOutAddr << endl;
					cout << endl << endl;

					emptyCnt = 0;
					int tmp;
					// sort the vectors according to their occurences  
					for (v = 0; v < w*h*CBf; v++)
						emptyCnt += nextCodebooksCnt[index][v] == 0;
					int *sortedIdx = (int*)xMalloc(int, w*h*CBf);
					int *newOrder = (int*)xMalloc(int, w*h*CBf);
					int *curOrder = (int*)xMalloc(int, w*h*CBf);
					for (v = 0; v < w*h*CBf; v++)
					{
						curOrder[v] = nextCodebooksCnt[index][v];
						sortedIdx[v] = v;
					}
					for (int bv1 = 0; bv1 < w*h*(CBf); bv1++)
						for (int bv2 = bv1; bv2 < w * h*CBf; bv2++)
						{
							if (curOrder[bv2] > curOrder[bv1])
							{
								// change
								tmp = curOrder[bv1];
								curOrder[bv1] = curOrder[bv2];
								curOrder[bv2] = tmp;

								tmp = sortedIdx[bv1];
								sortedIdx[bv1] = sortedIdx[bv2];
								sortedIdx[bv2] = tmp;
							}
						}
					emptyCnt = 0;

					for (v = 0; v < w*h*CBf; v++)
					{
						// cout << endl << endl << "v:" << v << endl << endl;
						emptyCnt += nextCodebooksCnt[index][v] == 0;
						for (r = 0; r < w*h; r++)
						{
							if (nextCodebooksCnt[index][v])
								fprintf(outCB, "%4d\t", nextCodebooks[index][v][r] / nextCodebooksCnt[index][v]);
							else
								fprintf(outCB, "%4d\t", Codebooks[index][sortedIdx[emptyCnt - 1]][r]);

							nextCodebooks[index][v][r] = 0;
						}

						fprintf(outCB, "%4d\n", nextCodebooksCnt[index][v]);
						nextCodebooksCnt[index][v] = 0;
					}
					fclose(outCB);
				}
			AllCUCount = 0;
			PIPValudCUCount = 0;
			PIPCUcount = 0;
			for (int line = 0; line < 5; line++)
			{
				for (int sym = 0; sym < 150; sym++)
					cout << "=";
				cout << endl;
			}
		}



		// print out CBF stats 
		fprintf(stdout, "JEM blocks count: %6d, CBF1 rate: %f\nPIP blocks count: %6d, CBF1 rate: %f\n", cbfStats[0], (double)cbfStats[2] / (double)cbfStats[0], cbfStats[1], (double)cbfStats[3] / (double)cbfStats[1]);
		fprintf(stdout, "Total PIP Bits: %f\nTotal All Bits: %f\n", totalPIPbits, totalAllbits);

		fout = fopen("log2.txt", "a");
		if (fout)
		{
			fprintf(fout, "\n Final: \n%10d\t%10d", (Int)totalPIPbits, (Int)totalAllbits);
			fclose(fout);
		}

#if SP_QCHANGE
		fprintf(stdout, "IsChanged rate: %1.3f\t - Change=1 rate: %1.3f\n", (float)spQisChanged_stat/float(spQall), (float)spQchStat/(float)spQisChanged_stat);
#endif
#endif

		// destroy application encoder class
		cTAppEncTop.destroy();

#if PIP
		// write the constructed codebook

	}
#endif
  return 0;
}

//! \}

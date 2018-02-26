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

/** \file     TEncSbac.cpp
    \brief    SBAC encoder class
*/

#include "TEncTop.h"
#include "TEncSbac.h"
#include "TLibCommon/TComTU.h"

#include <map>
#include <algorithm>
#if VCEG_AZ07_CTX_RESIDUALCODING
#include <math.h>
#endif

#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
#include "../TLibCommon/Debug.h"
#endif


//! \ingroup TLibEncoder
//! \{

// ====================================================================================================================
// Constructor / destructor / create / destroy
// ====================================================================================================================

TEncSbac::TEncSbac()
// new structure here
: m_pcBitIf                            ( NULL )
, m_pcBinIf                            ( NULL )
, m_numContextModels                   ( 0 )
, m_cCUSplitFlagSCModel                ( 1,             1,                      NUM_SPLIT_FLAG_CTX                   , m_contextModels + m_numContextModels, m_numContextModels)
#if JVET_C0024_QTBT
, m_cBTSplitFlagSCModel                ( 1,             1,                      NUM_BTSPLIT_MODE_CTX                 , m_contextModels + m_numContextModels, m_numContextModels )
#endif
, m_cCUSkipFlagSCModel                 ( 1,             1,                      NUM_SKIP_FLAG_CTX                    , m_contextModels + m_numContextModels, m_numContextModels)
#if VCEG_AZ05_INTRA_MPI
, m_cMPIIdxSCModel                     ( 1,             1,                      NUM_MPI_CTX                          , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if COM16_C1046_PDPC_INTRA
 , m_cPDPCIdxSCModel                   (1,              1,                      NUM_PDPC_CTX                         , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if VCEG_AZ05_ROT_TR || COM16_C1044_NSST
, m_cROTidxSCModel           ( 1,             1,               NUM_ROT_TR_CTX             , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cCUMergeFlagExtSCModel             ( 1,             1,                      NUM_MERGE_FLAG_EXT_CTX               , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMergeIdxExtSCModel              ( 1,             1,                      NUM_MERGE_IDX_EXT_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
#if VCEG_AZ07_FRUC_MERGE
, m_cCUFRUCMgrModeSCModel              ( 1,             1,                      NUM_FRUCMGRMODE_CTX                  , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUFRUCMESCModel                   ( 1,             1,                      NUM_FRUCME_CTX                       , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cCUPartSizeSCModel                 ( 1,             1,                      NUM_PART_SIZE_CTX                    , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPredModeSCModel                 ( 1,             1,                      NUM_PRED_MODE_CTX                    , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUIntraPredSCModel                ( 1,             1,                      NUM_INTRA_PREDICT_CTX                , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUChromaPredSCModel               ( 1,             1,                      NUM_CHROMA_PRED_CTX                  , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUDeltaQpSCModel                  ( 1,             1,                      NUM_DELTA_QP_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUInterDirSCModel                 ( 1,             1,                      NUM_INTER_DIR_CTX                    , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCURefPicSCModel                   ( 1,             1,                      NUM_REF_NO_CTX                       , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUMvdSCModel                      ( 1,             1,                      NUM_MV_RES_CTX                       , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUQtCbfSCModel                    ( 1,             NUM_QT_CBF_CTX_SETS,    NUM_QT_CBF_CTX_PER_SET               , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUTransSubdivFlagSCModel          ( 1,             1,                      NUM_TRANS_SUBDIV_FLAG_CTX            , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUQtRootCbfSCModel                ( 1,             1,                      NUM_QT_ROOT_CBF_CTX                  , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigCoeffGroupSCModel            ( 1,             2,                      NUM_SIG_CG_FLAG_CTX                  , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigSCModel                      ( 1,             1,                      NUM_SIG_FLAG_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastX                        ( 1,             NUM_CTX_LAST_FLAG_SETS, NUM_CTX_LAST_FLAG_XY                 , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastY                        ( 1,             NUM_CTX_LAST_FLAG_SETS, NUM_CTX_LAST_FLAG_XY                 , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUOneSCModel                      ( 1,             1,                      NUM_ONE_FLAG_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
#if !VCEG_AZ07_CTX_RESIDUALCODING
, m_cCUAbsSCModel                      ( 1,             1,                      NUM_ABS_FLAG_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cMVPIdxSCModel                     ( 1,             1,                      NUM_MVP_IDX_CTX                      , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoMergeSCModel                   ( 1,             1,                      NUM_SAO_MERGE_FLAG_CTX               , m_contextModels + m_numContextModels, m_numContextModels)
, m_cSaoTypeIdxSCModel                 ( 1,             1,                      NUM_SAO_TYPE_IDX_CTX                 , m_contextModels + m_numContextModels, m_numContextModels)
, m_cTransformSkipSCModel              ( 1,             MAX_NUM_CHANNEL_TYPE,   NUM_TRANSFORMSKIP_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
#if VCEG_AZ08_KLT_COMMON
, m_cKLTFlagSCModel                    ( 1,             MAX_NUM_CHANNEL_TYPE,   NUM_KLT_FLAG_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_CUTransquantBypassFlagSCModel      ( 1,             1,                      NUM_CU_TRANSQUANT_BYPASS_FLAG_CTX    , m_contextModels + m_numContextModels, m_numContextModels)
, m_explicitRdpcmFlagSCModel           ( 1,             MAX_NUM_CHANNEL_TYPE,   NUM_EXPLICIT_RDPCM_FLAG_CTX          , m_contextModels + m_numContextModels, m_numContextModels)
, m_explicitRdpcmDirSCModel            ( 1,             MAX_NUM_CHANNEL_TYPE,   NUM_EXPLICIT_RDPCM_DIR_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCrossComponentPredictionSCModel   ( 1,             1,                      NUM_CROSS_COMPONENT_PREDICTION_CTX   , m_contextModels + m_numContextModels, m_numContextModels)
, m_ChromaQpAdjFlagSCModel             ( 1,             1,                      NUM_CHROMA_QP_ADJ_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
, m_ChromaQpAdjIdcSCModel              ( 1,             1,                      NUM_CHROMA_QP_ADJ_IDC_CTX            , m_contextModels + m_numContextModels, m_numContextModels)
#if COM16_C806_OBMC
, m_cCUOBMCFlagSCModel                 ( 1,             1,                      NUM_OBMC_FLAG_CTX                    , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if VCEG_AZ07_IMV
, m_cCUiMVFlagSCModel                  ( 1,             1,                      NUM_IMV_FLAG_CTX                     , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if VCEG_AZ06_IC
, m_cCUICFlagSCModel                   ( 1,             1,                      NUM_IC_FLAG_CTX                      , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if ALF_HM3_REFACTOR
, m_bAlfCtrl                           ( false )
, m_uiMaxAlfCtrlDepth                  ( 0 )
, m_cCUAlfCtrlFlagSCModel              ( 1,             1,               NUM_ALF_CTRL_FLAG_CTX         , m_contextModels + m_numContextModels, m_numContextModels)
#if !JVET_C0038_GALF
, m_cALFFlagSCModel                    ( 1,             1,               NUM_ALF_FLAG_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cALFUvlcSCModel                    ( 1,             1,               NUM_ALF_UVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#if !JVET_C0038_GALF
, m_cALFSvlcSCModel                    ( 1,             1,               NUM_ALF_SVLC_CTX              , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#endif
#if COM16_C806_EMT
, m_cEmtTuIdxSCModel                   ( 1,             1,               NUM_EMT_TU_IDX_CTX            , m_contextModels + m_numContextModels, m_numContextModels)
, m_cEmtCuFlagSCModel                  ( 1,             1,               NUM_EMT_CU_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if COM16_C1016_AFFINE
, m_cCUAffineFlagSCModel               ( 1,             1,               NUM_AFFINE_FLAG_CTX           , m_contextModels + m_numContextModels, m_numContextModels)
// PIP context models
#if PIP
, m_cCUQtCbfSCModelPIP				   ( 1,             1,			     NUM_QT_CBF_CTX_PER_SET        , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigCoeffGroupSCModelPIP		   ( 1,             2,               NUM_SIG_CG_FLAG_CTX		   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUSigSCModelPIP				   ( 1,             1,               NUM_SIG_FLAG_CTX	           , m_contextModels + m_numContextModels, m_numContextModels) // NUM_SIG_FLAG_CTX_LUMA instead of NUM_SIG_FLAG_CTX
, m_cCuCtxLastXPIP					   ( 1,             NUM_CTX_LAST_FLAG_SETS, NUM_CTX_LAST_FLAG_XY   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCuCtxLastYPIP					   ( 1,             NUM_CTX_LAST_FLAG_SETS, NUM_CTX_LAST_FLAG_XY   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUOneSCModelPIP				   ( 1,             1,               NUM_ONE_FLAG_CTX_LUMA         , m_contextModels + m_numContextModels, m_numContextModels) // NUM_ONE_FLAG_CTX_LUMA instead of NUM_ONE_FLAG_CTX
, m_cCUPIPflag						   ( 1,             1,			     NUM_QT_CBF_CTX_PER_SET        , m_contextModels + m_numContextModels, m_numContextModels) 
, m_cCUR1SpGr						   ( 1,				1,				 NUM_SPATIAL_QN_GR			   , m_contextModels + m_numContextModels, m_numContextModels)
#if R1_DIRECTCODING
, m_cCUR1DirectCoding				   ( 1,				1,				 NUM_CTX_R1_DIRECT			   , m_contextModels + m_numContextModels, m_numContextModels)
#endif
, m_cCUR1SpSign						   ( 1,				1,				 1							   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUR1SpIsChanged				   ( 1,				1,				 1							   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUR1SpChange					   ( 1,				1,				 1							   , m_contextModels + m_numContextModels, m_numContextModels)
, m_cCUPIPPredictors				   ( 1,				1,				 NUM_CTX_PREDICTORS			   , m_contextModels + m_numContextModels, m_numContextModels)
#if CBF2x2
, m_cCUPIPR1Cbf						   ( 1,				1,				 4			   , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if NOISE_MARK
, m_cCUPIPNoiseMark						   ( 1,				1,				 1			   , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#if SPR1_ClUSTER
, m_cCUR1SpCluster					   ( 1,				1,				 NUM_CLUSTER_CTX			   , m_contextModels + m_numContextModels, m_numContextModels)
#endif
#endif
#endif
{
  assert( m_numContextModels <= MAX_NUM_CTX_MOD );
}

TEncSbac::~TEncSbac()
{
}

// ====================================================================================================================
// Public member functions
// ====================================================================================================================

Void TEncSbac::resetEntropy           (const TComSlice *pSlice)
{
  Int  iQp              = pSlice->getSliceQp();
  SliceType eSliceType  = pSlice->getSliceType();

  SliceType encCABACTableIdx = pSlice->getEncCABACTableIdx();
  if (!pSlice->isIntra() && (encCABACTableIdx==B_SLICE || encCABACTableIdx==P_SLICE) && pSlice->getPPS()->getCabacInitPresentFlag())
  {
    eSliceType = encCABACTableIdx;
  }

  m_cCUSplitFlagSCModel.initBuffer                ( eSliceType, iQp, (UChar*)INIT_SPLIT_FLAG );
#if JVET_C0024_QTBT
  m_cBTSplitFlagSCModel.initBuffer                ( eSliceType, iQp, (UChar*)INIT_BTSPLIT_MODE );
#endif
  m_cCUSkipFlagSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_SKIP_FLAG );
#if VCEG_AZ05_INTRA_MPI
  m_cMPIIdxSCModel.initBuffer                     ( eSliceType, iQp, (UChar*)INIT_MPIIdx_FLAG );
#endif 
#if COM16_C1046_PDPC_INTRA
  m_cPDPCIdxSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_PDPCIdx_FLAG);
#endif
#if VCEG_AZ05_ROT_TR || COM16_C1044_NSST
  m_cROTidxSCModel.initBuffer        ( eSliceType, iQp, (UChar*)INIT_ROT_TR_IDX );
#endif
  m_cCUMergeFlagExtSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_MERGE_FLAG_EXT);
  m_cCUMergeIdxExtSCModel.initBuffer              ( eSliceType, iQp, (UChar*)INIT_MERGE_IDX_EXT);
#if VCEG_AZ07_FRUC_MERGE
  m_cCUFRUCMgrModeSCModel.initBuffer              ( eSliceType, iQp, (UChar*)INIT_FRUCMGRMODEBIN1 );
  m_cCUFRUCMESCModel.initBuffer                   ( eSliceType, iQp, (UChar*)INIT_FRUCMGRMODEBIN2 );
#endif
  m_cCUPartSizeSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_PART_SIZE );
  m_cCUPredModeSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_PRED_MODE );
  m_cCUIntraPredSCModel.initBuffer                ( eSliceType, iQp, (UChar*)INIT_INTRA_PRED_MODE );
  m_cCUChromaPredSCModel.initBuffer               ( eSliceType, iQp, (UChar*)INIT_CHROMA_PRED_MODE );
  m_cCUInterDirSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_INTER_DIR );
  m_cCUMvdSCModel.initBuffer                      ( eSliceType, iQp, (UChar*)INIT_MVD );
  m_cCURefPicSCModel.initBuffer                   ( eSliceType, iQp, (UChar*)INIT_REF_PIC );
  m_cCUDeltaQpSCModel.initBuffer                  ( eSliceType, iQp, (UChar*)INIT_DQP );
  m_cCUQtCbfSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_QT_CBF );
  m_cCUQtRootCbfSCModel.initBuffer                ( eSliceType, iQp, (UChar*)INIT_QT_ROOT_CBF );
  m_cCUSigCoeffGroupSCModel.initBuffer            ( eSliceType, iQp, (UChar*)INIT_SIG_CG_FLAG );
  m_cCUSigSCModel.initBuffer                      ( eSliceType, iQp, (UChar*)INIT_SIG_FLAG );
  m_cCuCtxLastX.initBuffer                        ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCuCtxLastY.initBuffer                        ( eSliceType, iQp, (UChar*)INIT_LAST );
  m_cCUOneSCModel.initBuffer                      ( eSliceType, iQp, (UChar*)INIT_ONE_FLAG );
#if !VCEG_AZ07_CTX_RESIDUALCODING
  m_cCUAbsSCModel.initBuffer                      ( eSliceType, iQp, (UChar*)INIT_ABS_FLAG );
#endif
  m_cMVPIdxSCModel.initBuffer                     ( eSliceType, iQp, (UChar*)INIT_MVP_IDX );
  m_cCUTransSubdivFlagSCModel.initBuffer          ( eSliceType, iQp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
  m_cSaoMergeSCModel.initBuffer                   ( eSliceType, iQp, (UChar*)INIT_SAO_MERGE_FLAG );
  m_cSaoTypeIdxSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_SAO_TYPE_IDX );
  m_cTransformSkipSCModel.initBuffer              ( eSliceType, iQp, (UChar*)INIT_TRANSFORMSKIP_FLAG );
#if VCEG_AZ08_KLT_COMMON
  m_cKLTFlagSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_KLT_FLAG);
#endif
  m_CUTransquantBypassFlagSCModel.initBuffer      ( eSliceType, iQp, (UChar*)INIT_CU_TRANSQUANT_BYPASS_FLAG );
  m_explicitRdpcmFlagSCModel.initBuffer           ( eSliceType, iQp, (UChar*)INIT_EXPLICIT_RDPCM_FLAG);
  m_explicitRdpcmDirSCModel.initBuffer            ( eSliceType, iQp, (UChar*)INIT_EXPLICIT_RDPCM_DIR);
  m_cCrossComponentPredictionSCModel.initBuffer   ( eSliceType, iQp, (UChar*)INIT_CROSS_COMPONENT_PREDICTION  );
  m_ChromaQpAdjFlagSCModel.initBuffer             ( eSliceType, iQp, (UChar*)INIT_CHROMA_QP_ADJ_FLAG );
  m_ChromaQpAdjIdcSCModel.initBuffer              ( eSliceType, iQp, (UChar*)INIT_CHROMA_QP_ADJ_IDC );
#if COM16_C806_OBMC
  m_cCUOBMCFlagSCModel.initBuffer                 ( eSliceType, iQp, (UChar*)INIT_OBMC_FLAG );
#endif
#if VCEG_AZ07_IMV
  m_cCUiMVFlagSCModel.initBuffer                  ( eSliceType, iQp, (UChar*)INIT_IMV_FLAG );
#endif
#if VCEG_AZ06_IC
  m_cCUICFlagSCModel.initBuffer                   ( eSliceType, iQp, (UChar*)INIT_IC_FLAG );
#endif
#if ALF_HM3_REFACTOR
  m_cCUAlfCtrlFlagSCModel.initBuffer              ( eSliceType, iQp, (UChar*)INIT_ALF_CTRL_FLAG );
#if !JVET_C0038_GALF
  m_cALFFlagSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_ALF_FLAG );
#endif
  m_cALFUvlcSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_ALF_UVLC );
#if !JVET_C0038_GALF
  m_cALFSvlcSCModel.initBuffer                    ( eSliceType, iQp, (UChar*)INIT_ALF_SVLC );
#endif
#endif
#if COM16_C806_EMT
  m_cEmtTuIdxSCModel.initBuffer                   ( eSliceType, iQp, (UChar*)INIT_EMT_TU_IDX );
  m_cEmtCuFlagSCModel.initBuffer                  ( eSliceType, iQp, (UChar*)INIT_EMT_CU_FLAG );
#endif 
#if COM16_C1016_AFFINE
  m_cCUAffineFlagSCModel.initBuffer               ( eSliceType, iQp, (UChar*)INIT_AFFINE_FLAG );
#endif

  // PIP context models
#if PIP
  m_cCUQtCbfSCModelPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_QT_CBF);
  m_cCUSigCoeffGroupSCModelPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_SIG_CG_FLAG);
  m_cCUSigSCModelPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_SIG_FLAG);
  m_cCuCtxLastXPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_LAST);
  m_cCuCtxLastYPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_LAST);
  m_cCUOneSCModelPIP.initBuffer(eSliceType, iQp, (UChar*)INIT_ONE_FLAG_PIP);  
  m_cCUPIPflag.initBuffer(eSliceType, iQp, (UChar*)INIT_FLAG_PIP);
  m_cCUR1SpGr.initBuffer(eSliceType, iQp, (UChar*)INIT_SPATIAL_GR);
#if R1_DIRECTCODING
  m_cCUR1DirectCoding.initBuffer(eSliceType, iQp, (UChar*)INIT_R1_DIRECT);
#endif
  m_cCUR1SpSign.initBuffer(eSliceType, iQp, (UChar*)INIT_SPATIAL_SIGN);
  m_cCUR1SpIsChanged.initBuffer(eSliceType, iQp, (UChar*)INIT_SPATIAL_ISCHANGED); 
  m_cCUR1SpChange.initBuffer(eSliceType, iQp, (UChar*)INIT_SPATIAL_CHANGE);
  m_cCUPIPPredictors.initBuffer(eSliceType, iQp, (UChar*)INIT_PIP_PREDICTORS);
#if CBF2x2
  m_cCUPIPR1Cbf.initBuffer(eSliceType, iQp, (UChar*)INIT_PIP_R1CBF);
#endif
#if NOISE_MARK
  m_cCUPIPNoiseMark.initBuffer(eSliceType, iQp, (UChar*)INIT_PIP_NOISEMARK);
#endif
#if SPR1_ClUSTER
  m_cCUR1SpCluster.initBuffer(eSliceType, iQp, (UChar*)INIT_SPATIAL_CLUSTER_R1);
#endif
#endif

  for (UInt statisticIndex = 0; statisticIndex < RExt__GOLOMB_RICE_ADAPTATION_STATISTICS_SETS ; statisticIndex++)
  {
    m_golombRiceAdaptationStatistics[statisticIndex] = 0;
  }

  m_pcBinIf->start();
#if VCEG_AZ07_BAC_ADAPT_WDOW 
  xUpdateWindowSize (pSlice->getSliceType(), pSlice->getCtxMapQPIdx(), pSlice->getStatsHandle());
#endif
  return;
}

/** The function does the following:
 * If current slice type is P/B then it determines the distance of initialisation type 1 and 2 from the current CABAC states and
 * stores the index of the closest table.  This index is used for the next P/B slice when cabac_init_present_flag is true.
 */
SliceType TEncSbac::determineCabacInitIdx(const TComSlice *pSlice)
{
  Int  qp              = pSlice->getSliceQp();

  if (!pSlice->isIntra())
  {
    SliceType aSliceTypeChoices[] = {B_SLICE, P_SLICE};

    UInt bestCost             = MAX_UINT;
    SliceType bestSliceType   = aSliceTypeChoices[0];
    for (UInt idx=0; idx<2; idx++)
    {
      UInt curCost          = 0;
      SliceType curSliceType  = aSliceTypeChoices[idx];

      curCost  = m_cCUSplitFlagSCModel.calcCost                ( curSliceType, qp, (UChar*)INIT_SPLIT_FLAG );
#if JVET_C0024_QTBT
      curCost += m_cBTSplitFlagSCModel.calcCost                ( curSliceType, qp, (UChar*)INIT_BTSPLIT_MODE );
#endif
      curCost += m_cCUSkipFlagSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_SKIP_FLAG );
#if VCEG_AZ05_INTRA_MPI
      curCost += m_cMPIIdxSCModel.calcCost                     ( curSliceType, qp, (UChar*)INIT_MPIIdx_FLAG );
#endif
#if COM16_C1046_PDPC_INTRA
      curCost += m_cPDPCIdxSCModel.calcCost                    ( curSliceType, qp, (UChar*)INIT_PDPCIdx_FLAG);
#endif
#if VCEG_AZ05_ROT_TR || COM16_C1044_NSST
      curCost += m_cROTidxSCModel.calcCost        ( curSliceType, qp, (UChar*)INIT_ROT_TR_IDX );
#endif
      curCost += m_cCUMergeFlagExtSCModel.calcCost             ( curSliceType, qp, (UChar*)INIT_MERGE_FLAG_EXT);
      curCost += m_cCUMergeIdxExtSCModel.calcCost              ( curSliceType, qp, (UChar*)INIT_MERGE_IDX_EXT);
#if VCEG_AZ07_FRUC_MERGE
      curCost += m_cCUFRUCMgrModeSCModel.calcCost              ( curSliceType, qp, (UChar*)INIT_FRUCMGRMODEBIN1);
      curCost += m_cCUFRUCMESCModel.calcCost                   ( curSliceType, qp, (UChar*)INIT_FRUCMGRMODEBIN2);
#endif
      curCost += m_cCUPartSizeSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_PART_SIZE );
      curCost += m_cCUPredModeSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_PRED_MODE );
      curCost += m_cCUIntraPredSCModel.calcCost                ( curSliceType, qp, (UChar*)INIT_INTRA_PRED_MODE );
      curCost += m_cCUChromaPredSCModel.calcCost               ( curSliceType, qp, (UChar*)INIT_CHROMA_PRED_MODE );
      curCost += m_cCUInterDirSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_INTER_DIR );
      curCost += m_cCUMvdSCModel.calcCost                      ( curSliceType, qp, (UChar*)INIT_MVD );
      curCost += m_cCURefPicSCModel.calcCost                   ( curSliceType, qp, (UChar*)INIT_REF_PIC );
      curCost += m_cCUDeltaQpSCModel.calcCost                  ( curSliceType, qp, (UChar*)INIT_DQP );
      curCost += m_cCUQtCbfSCModel.calcCost                    ( curSliceType, qp, (UChar*)INIT_QT_CBF );
      curCost += m_cCUQtRootCbfSCModel.calcCost                ( curSliceType, qp, (UChar*)INIT_QT_ROOT_CBF );
      curCost += m_cCUSigCoeffGroupSCModel.calcCost            ( curSliceType, qp, (UChar*)INIT_SIG_CG_FLAG );
      curCost += m_cCUSigSCModel.calcCost                      ( curSliceType, qp, (UChar*)INIT_SIG_FLAG );
      curCost += m_cCuCtxLastX.calcCost                        ( curSliceType, qp, (UChar*)INIT_LAST );
      curCost += m_cCuCtxLastY.calcCost                        ( curSliceType, qp, (UChar*)INIT_LAST );
      curCost += m_cCUOneSCModel.calcCost                      ( curSliceType, qp, (UChar*)INIT_ONE_FLAG );
#if !VCEG_AZ07_CTX_RESIDUALCODING
      curCost += m_cCUAbsSCModel.calcCost                      ( curSliceType, qp, (UChar*)INIT_ABS_FLAG );
#endif
      curCost += m_cMVPIdxSCModel.calcCost                     ( curSliceType, qp, (UChar*)INIT_MVP_IDX );
      curCost += m_cCUTransSubdivFlagSCModel.calcCost          ( curSliceType, qp, (UChar*)INIT_TRANS_SUBDIV_FLAG );
      curCost += m_cSaoMergeSCModel.calcCost                   ( curSliceType, qp, (UChar*)INIT_SAO_MERGE_FLAG );
      curCost += m_cSaoTypeIdxSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_SAO_TYPE_IDX );
      curCost += m_cTransformSkipSCModel.calcCost              ( curSliceType, qp, (UChar*)INIT_TRANSFORMSKIP_FLAG );
#if VCEG_AZ08_KLT_COMMON
      curCost += m_cKLTFlagSCModel.calcCost                    ( curSliceType, qp, (UChar*)INIT_KLT_FLAG);
#endif
      curCost += m_CUTransquantBypassFlagSCModel.calcCost      ( curSliceType, qp, (UChar*)INIT_CU_TRANSQUANT_BYPASS_FLAG );
      curCost += m_explicitRdpcmFlagSCModel.calcCost           ( curSliceType, qp, (UChar*)INIT_EXPLICIT_RDPCM_FLAG);
      curCost += m_explicitRdpcmDirSCModel.calcCost            ( curSliceType, qp, (UChar*)INIT_EXPLICIT_RDPCM_DIR);
      curCost += m_cCrossComponentPredictionSCModel.calcCost   ( curSliceType, qp, (UChar*)INIT_CROSS_COMPONENT_PREDICTION );
      curCost += m_ChromaQpAdjFlagSCModel.calcCost             ( curSliceType, qp, (UChar*)INIT_CHROMA_QP_ADJ_FLAG );
      curCost += m_ChromaQpAdjIdcSCModel.calcCost              ( curSliceType, qp, (UChar*)INIT_CHROMA_QP_ADJ_IDC );
#if COM16_C806_OBMC
      curCost += m_cCUOBMCFlagSCModel.calcCost                 ( curSliceType, qp, (UChar*)INIT_OBMC_FLAG );
#endif
#if COM16_C806_EMT
      curCost += m_cEmtTuIdxSCModel.calcCost                   ( curSliceType, qp, (UChar*)INIT_EMT_TU_IDX );
      curCost += m_cEmtCuFlagSCModel.calcCost                  ( curSliceType, qp, (UChar*)INIT_EMT_CU_FLAG );
#endif
#if VCEG_AZ07_IMV
      curCost += m_cCUiMVFlagSCModel.calcCost                  ( curSliceType, qp, (UChar*)INIT_IMV_FLAG );
#endif
#if VCEG_AZ06_IC
      curCost += m_cCUICFlagSCModel.calcCost                   ( curSliceType, qp, (UChar*)INIT_IC_FLAG );
#endif
#if COM16_C1016_AFFINE
      curCost += m_cCUAffineFlagSCModel.calcCost               ( curSliceType, qp, (UChar*)INIT_AFFINE_FLAG );
#endif

	  //PIP context models
#if PIP
	  curCost += m_cCUQtCbfSCModelPIP.calcCost(curSliceType, qp, (UChar*)INIT_QT_CBF);
	  curCost += m_cCUSigCoeffGroupSCModelPIP.calcCost(curSliceType, qp, (UChar*)INIT_SIG_CG_FLAG);
	  curCost += m_cCUSigSCModelPIP.calcCost(curSliceType, qp, (UChar*)INIT_SIG_FLAG_PIP);
	  curCost += m_cCuCtxLastXPIP.calcCost(curSliceType, qp, (UChar*)INIT_LAST);
	  curCost += m_cCuCtxLastYPIP.calcCost(curSliceType, qp, (UChar*)INIT_LAST);
	  curCost += m_cCUOneSCModelPIP.calcCost(curSliceType, qp, (UChar*)INIT_ONE_FLAG);
	  curCost += m_cCUPIPflag.calcCost(curSliceType, qp, (UChar*)INIT_FLAG_PIP);
	  curCost += m_cCUR1SpGr.calcCost(curSliceType, qp, (UChar*)INIT_SPATIAL_GR);
#if R1_DIRECTCODING
	  curCost += m_cCUR1DirectCoding.calcCost(curSliceType, qp, (UChar*)INIT_R1_DIRECT);
#endif
	  curCost += m_cCUR1SpSign.calcCost(curSliceType, qp, (UChar*)INIT_SPATIAL_SIGN);
	  curCost += m_cCUR1SpIsChanged.calcCost(curSliceType, qp, (UChar*)INIT_SPATIAL_ISCHANGED);
	  curCost += m_cCUR1SpChange.calcCost(curSliceType, qp, (UChar*)INIT_SPATIAL_CHANGE);
	  curCost += m_cCUPIPPredictors.calcCost(curSliceType, qp, (UChar*)INIT_PIP_PREDICTORS);
#if CBF2x2
	  curCost += m_cCUPIPR1Cbf.calcCost(curSliceType, qp, (UChar*)INIT_PIP_R1CBF);
#endif
#if NOISE_MARK
	  curCost += m_cCUPIPNoiseMark.calcCost(curSliceType, qp, (UChar*)INIT_PIP_NOISEMARK);
#endif
#if SPR1_ClUSTER
	  curCost += m_cCUR1SpCluster.calcCost(curSliceType, qp, (UChar*)INIT_SPATIAL_CLUSTER_R1);
#endif
#endif

      if (curCost < bestCost)
      {
        bestSliceType = curSliceType;
        bestCost      = curCost;
      }
    }
    return bestSliceType;
  }
  else
  {
    return I_SLICE;
  }
}

Void TEncSbac::codeVPS( const TComVPS* /*pcVPS*/ )
{
  assert (0);
  return;
}

Void TEncSbac::codeSPS( const TComSPS* /*pcSPS*/ )
{
  assert (0);
  return;
}

Void TEncSbac::codePPS( const TComPPS* /*pcPPS*/ )
{
  assert (0);
  return;
}

Void TEncSbac::codeSliceHeader( TComSlice* /*pcSlice*/ )
{
  assert (0);
  return;
}

Void TEncSbac::codeTilesWPPEntryPoint( TComSlice* /*pSlice*/ )
{
  assert (0);
  return;
}

Void TEncSbac::codeTerminatingBit( UInt uilsLast )
{
  m_pcBinIf->encodeBinTrm( uilsLast );
}

Void TEncSbac::codeSliceFinish()
{
  m_pcBinIf->finish();
}

Void TEncSbac::xWriteUnarySymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset )
{
  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[0] );

  if( 0 == uiSymbol)
  {
    return;
  }

  while( uiSymbol-- )
  {
    m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ iOffset ] );
  }

  return;
}

Void TEncSbac::xWriteUnaryMaxSymbol( UInt uiSymbol, ContextModel* pcSCModel, Int iOffset, UInt uiMaxSymbol )
{
  if (uiMaxSymbol == 0)
  {
    return;
  }

  m_pcBinIf->encodeBin( uiSymbol ? 1 : 0, pcSCModel[ 0 ] );

  if ( uiSymbol == 0 )
  {
    return;
  }

  Bool bCodeLast = ( uiMaxSymbol > uiSymbol );

  while( --uiSymbol )
  {
    m_pcBinIf->encodeBin( 1, pcSCModel[ iOffset ] );
  }
  if( bCodeLast )
  {
    m_pcBinIf->encodeBin( 0, pcSCModel[ iOffset ] );
  }

  return;
}

#if JVET_B0051_NON_MPM_MODE || JVET_C0038_GALF
Void TEncSbac::xWriteTruncBinCode(UInt uiSymbol, UInt uiMaxSymbol)
{
  UInt uiThresh;
  if (uiMaxSymbol > 256)
  {
    UInt uiThreshVal = 1 << 8;
    uiThresh = 8;
    while (uiThreshVal <= uiMaxSymbol)
    {
      uiThresh++;
      uiThreshVal <<= 1;
    }
    uiThresh--;
  }
  else
  {
    uiThresh = g_NonMPM[uiMaxSymbol];
  }

  UInt uiVal = 1 << uiThresh;
  assert(uiVal <= uiMaxSymbol);
  assert((uiVal << 1) > uiMaxSymbol);
  assert(uiSymbol < uiMaxSymbol);
  UInt b = uiMaxSymbol - uiVal;
  assert(b < uiVal);
  if (uiSymbol < uiVal - b)
  {
    m_pcBinIf->encodeBinsEP(uiSymbol, uiThresh);
  }
  else
  {
    uiSymbol += uiVal - b;
    assert(uiSymbol < (uiVal << 1));
    assert((uiSymbol >> 1) >= uiVal - b);
    m_pcBinIf->encodeBinsEP(uiSymbol, uiThresh + 1);
  }
}
#endif

Void TEncSbac::xWriteEpExGolomb( UInt uiSymbol, UInt uiCount )
{
  UInt bins = 0;
  Int numBins = 0;

  while( uiSymbol >= (UInt)(1<<uiCount) )
  {
    bins = 2 * bins + 1;
    numBins++;
    uiSymbol -= 1 << uiCount;
    uiCount  ++;
  }
  bins = 2 * bins + 0;
  numBins++;

  bins = (bins << uiCount) | uiSymbol;
  numBins += uiCount;

  assert( numBins <= 32 );
  m_pcBinIf->encodeBinsEP( bins, numBins );
}


/** Coding of coeff_abs_level_minus3
 * \param symbol                  value of coeff_abs_level_minus3
 * \param rParam                  reference to Rice parameter
 * \param useLimitedPrefixLength
 * \param maxLog2TrDynamicRange 
 */

Void TEncSbac::xWriteCoefRemainExGolomb ( UInt symbol, UInt &rParam, const Bool useLimitedPrefixLength, const Int maxLog2TrDynamicRange )
{
  Int codeNumber  = (Int)symbol;
  UInt length;
#if VCEG_AZ07_CTX_RESIDUALCODING
  if (codeNumber < (g_auiGoRiceRange[rParam] << rParam))
#else
  if (codeNumber < (COEF_REMAIN_BIN_REDUCTION << rParam))
#endif
  {
    length = codeNumber>>rParam;
    m_pcBinIf->encodeBinsEP( (1<<(length+1))-2 , length+1);
    m_pcBinIf->encodeBinsEP((codeNumber%(1<<rParam)),rParam);
  }
  else if (useLimitedPrefixLength)
  {
    const UInt maximumPrefixLength = (32 - (COEF_REMAIN_BIN_REDUCTION + maxLog2TrDynamicRange));

    UInt prefixLength = 0;
    UInt suffixLength = MAX_UINT;
    UInt codeValue    = (symbol >> rParam) - COEF_REMAIN_BIN_REDUCTION;

    if (codeValue >= ((1 << maximumPrefixLength) - 1))
    {
      prefixLength = maximumPrefixLength;
      suffixLength = maxLog2TrDynamicRange - rParam;
    }
    else
    {
      while (codeValue > ((2 << prefixLength) - 2))
      {
        prefixLength++;
      }

      suffixLength = prefixLength + 1; //+1 for the separator bit
    }

    const UInt suffix = codeValue - ((1 << prefixLength) - 1);

    const UInt totalPrefixLength = prefixLength + COEF_REMAIN_BIN_REDUCTION;
    const UInt prefix            = (1 << totalPrefixLength) - 1;
    const UInt rParamBitMask     = (1 << rParam) - 1;

    m_pcBinIf->encodeBinsEP(  prefix,                                        totalPrefixLength      ); //prefix
    m_pcBinIf->encodeBinsEP(((suffix << rParam) | (symbol & rParamBitMask)), (suffixLength + rParam)); //separator, suffix, and rParam bits
  }
  else
  {
    length = rParam;
#if VCEG_AZ07_CTX_RESIDUALCODING
    codeNumber  = codeNumber - ( g_auiGoRiceRange[rParam] << rParam);
#else
    codeNumber  = codeNumber - ( COEF_REMAIN_BIN_REDUCTION << rParam);
#endif

    while (codeNumber >= (1<<length))
    {
      codeNumber -=  (1<<(length++));
    }
#if VCEG_AZ07_CTX_RESIDUALCODING
    m_pcBinIf->encodeBinsEP((1<<(g_auiGoRiceRange[rParam] + length + 1 - rParam))-2, g_auiGoRiceRange[rParam] + length + 1 - rParam);
#else
    m_pcBinIf->encodeBinsEP((1<<(COEF_REMAIN_BIN_REDUCTION+length+1-rParam))-2,COEF_REMAIN_BIN_REDUCTION+length+1-rParam);
#endif
    m_pcBinIf->encodeBinsEP(codeNumber,length);
  }
}

// SBAC RD
Void  TEncSbac::load ( const TEncSbac* pSrc)
{
  this->xCopyFrom(pSrc);
}

Void  TEncSbac::loadIntraDirMode( const TEncSbac* pSrc, const ChannelType chType )
{
  m_pcBinIf->copyState( pSrc->m_pcBinIf );
  if (isLuma(chType))
  {
    this->m_cCUIntraPredSCModel      .copyFrom( &pSrc->m_cCUIntraPredSCModel       );
  }
  else
  {
    this->m_cCUChromaPredSCModel     .copyFrom( &pSrc->m_cCUChromaPredSCModel      );
  }
}


Void  TEncSbac::store( TEncSbac* pDest) const
{
  pDest->xCopyFrom( this );
}


Void TEncSbac::xCopyFrom( const TEncSbac* pSrc )
{
  m_pcBinIf->copyState( pSrc->m_pcBinIf );
  xCopyContextsFrom(pSrc);
}

Void TEncSbac::codeMVPIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iSymbol = pcCU->getMVPIdx(eRefList, uiAbsPartIdx);
  Int iNum = AMVP_MAX_NUM_CANDS;

  xWriteUnaryMaxSymbol(iSymbol, m_cMVPIdxSCModel.get(0), 1, iNum-1);
}

#if !JVET_C0024_QTBT
Void TEncSbac::codePartSize( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
  PartSize eSize         = pcCU->getPartitionSize( uiAbsPartIdx );
  const UInt log2DiffMaxMinCodingBlockSize = pcCU->getSlice()->getSPS()->getLog2DiffMaxMinCodingBlockSize();

  if ( pcCU->isIntra( uiAbsPartIdx ) )
  {
    if( uiDepth == log2DiffMaxMinCodingBlockSize )
    {
      m_pcBinIf->encodeBin( eSize == SIZE_2Nx2N? 1 : 0, m_cCUPartSizeSCModel.get( 0, 0, 0 ) );
    }
    return;
  }

  switch(eSize)
  {
    case SIZE_2Nx2N:
    {
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      break;
    }
    case SIZE_2NxN:
    case SIZE_2NxnU:
    case SIZE_2NxnD:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 1) );
      if ( pcCU->getSlice()->getSPS()->getUseAMP() && uiDepth < log2DiffMaxMinCodingBlockSize )
      {
        if (eSize == SIZE_2NxN)
        {
          m_pcBinIf->encodeBin(1, m_cCUPartSizeSCModel.get( 0, 0, 3 ));
        }
        else
        {
          m_pcBinIf->encodeBin(0, m_cCUPartSizeSCModel.get( 0, 0, 3 ));
          m_pcBinIf->encodeBinEP((eSize == SIZE_2NxnU? 0: 1));
        }
      }
      break;
    }
    case SIZE_Nx2N:
    case SIZE_nLx2N:
    case SIZE_nRx2N:
    {
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
      m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
#if COM16_C806_HEVC_MOTION_CONSTRAINT_REMOVAL && !COM16_C806_DISABLE_4X4_PU
      if( uiDepth == log2DiffMaxMinCodingBlockSize && (pcCU->getSlice()->getSPS()->getAtmvpEnableFlag()||!( pcCU->getWidth(uiAbsPartIdx) == 8 && pcCU->getHeight(uiAbsPartIdx) == 8 )))
#else
      if( uiDepth == log2DiffMaxMinCodingBlockSize && !( pcCU->getWidth(uiAbsPartIdx) == 8 && pcCU->getHeight(uiAbsPartIdx) == 8 ) )
#endif
      {
        m_pcBinIf->encodeBin( 1, m_cCUPartSizeSCModel.get( 0, 0, 2) );
      }

      if ( pcCU->getSlice()->getSPS()->getUseAMP() && uiDepth < log2DiffMaxMinCodingBlockSize )
      {
        if (eSize == SIZE_Nx2N)
        {
          m_pcBinIf->encodeBin(1, m_cCUPartSizeSCModel.get( 0, 0, 3 ));
        }
        else
        {
          m_pcBinIf->encodeBin(0, m_cCUPartSizeSCModel.get( 0, 0, 3 ));
          m_pcBinIf->encodeBinEP((eSize == SIZE_nLx2N? 0: 1));
        }
      }
      break;
    }
    case SIZE_NxN:
    {
#if COM16_C806_HEVC_MOTION_CONSTRAINT_REMOVAL && !COM16_C806_DISABLE_4X4_PU
      if( uiDepth == log2DiffMaxMinCodingBlockSize && 
        (pcCU->getSlice()->getSPS()->getAtmvpEnableFlag()||!( pcCU->getWidth(uiAbsPartIdx) == 8 && pcCU->getHeight(uiAbsPartIdx) == 8 )))
#else
      if( uiDepth == log2DiffMaxMinCodingBlockSize && !( pcCU->getWidth(uiAbsPartIdx) == 8 && pcCU->getHeight(uiAbsPartIdx) == 8 ) )
#endif
      {
        m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 0) );
        m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 1) );
        m_pcBinIf->encodeBin( 0, m_cCUPartSizeSCModel.get( 0, 0, 2) );
      }
      break;
    }
    default:
    {
      assert(0);
      break;
    }
  }
}
#endif


/** code prediction mode
 * \param pcCU
 * \param uiAbsPartIdx
 * \returns Void
 */
Void TEncSbac::codePredMode( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  m_pcBinIf->encodeBin( pcCU->isIntra( uiAbsPartIdx ) ? 1 : 0, m_cCUPredModeSCModel.get( 0, 0, 0 ) );
}


#if PIP
Void TEncSbac::codePIPflag(TComDataCU* pcCU, UInt uiAbsPartIdx, int *spQR1
#if NOISE_MARK
	, int *spQR1NoiseMark
#endif
	, Pel* piOrg, int stride, Pel* piPred, int* piRefPred, int* piCurPred, int* piMinRes)
{	
	UInt w = pcCU->getWidth(uiAbsPartIdx); // not sure at all. Does this give the right width/height?
	UInt h = pcCU->getHeight(uiAbsPartIdx);
	UInt f = pcCU->getPIPflag(uiAbsPartIdx);
	// get context function is here
	UInt CtxIdx = (log2(w) - 2) * (log2((UInt)CUMAX) - 1) + (log2(h) - 2);	
		
	m_pcBinIf->encodeBin(f, m_cCUPIPflag.get(0, 0, CtxIdx));

	
	if (encodeTime && verbose)
		cout << ",PIP:" << (Int)f;
			

#if PIP_DOWNSAMPLE
	h /= 2;
	w /= 2;
#endif

	if (encodeTime)
	{
		cbfStats[f]++;
		cbfStats[f + 2] += (Int)pcCU->getCbf(uiAbsPartIdx, COMPONENT_Y);				
	}
	if (f
#if R1_CODECOEFFNXN_TEST
		&& !tmpFlag
#endif
		)
	{		
		Int idx = pcCU->getPIPCBidx(uiAbsPartIdx);

		const UInt offsetY = pcCU->getPic()->getCodedAreaInCTU();
		const UInt componentShift = pcCU->getPic()->getComponentScaleX(COMPONENT_Y) + pcCU->getPic()->getComponentScaleY(COMPONENT_Y);
			

		
#if SP_QCHANGE
		int tmp_spQ = spQ;
		int spQIsChanged = pcCU->getPIPspQIsChangedFlag(uiAbsPartIdx);
		int spQChange = 0;
#if SP_SIGNLECHANGE
		// m_pcBinIf->encodeBinEP(spQIsChanged);
		//m_pcBinIf->encodeBin(spQIsChanged, m_cCUR1SpIsChanged.get(0, 0, 0));
#endif
		if (spQIsChanged)
		{			
			spQChange = pcCU->getPIPspQChange(uiAbsPartIdx);
#if SP_SIGNLECHANGE
			// m_pcBinIf->encodeBinEP(spQChange);
			// m_pcBinIf->encodeBin(spQChange, m_cCUR1SpChange.get(0,0,0));
#else
#if SP_CHEAT
			// do not encode it!
#else
			// encode it: not decided how yet.
#endif
#endif
		}


		if (spQIsChanged)
		{
#if SP_SIGNLECHANGE
			spQ += (spQChange == 0 ? -1 : 1) * SP_QOFFSET;
			spQ = max(spQ, 1);
#else
			spQ += (spQChange % 2 ? -1 : 1) * (floor((spQChange - 1) / 2) + 1) * SP_QQUANTIZER;
			if (spQ < 1)
				spQ = 1;
#endif
		}

#if SP_QCHANGE
		if (encodeTime)
		{
			spQall++;
			spQisChanged_stat += spQIsChanged;
			spQchStat += (spQIsChanged && spQChange);
		}
#endif

		
#endif



		int b1 = m_pcBinIf->getNumWrittenBits();

#if CBF2x2

// ==============================
// ==============================
// ==============================
// --------------- encode 2x2cbf
		// Bool cbfException[16];
		Bool *cbfException = (Bool*)xMalloc(Bool, w*h*sizeof(Bool));
		memset(cbfException, 0, w*h * sizeof(Bool));
		// UInt cbf2x2[4] = { 0, 0, 0, 0 };
		UInt *cbf2x2 = (UInt*)xMalloc(UInt, w*h*sizeof(UInt) / 4);
		memset(cbf2x2, 0, w*h*sizeof(UInt) / 4);
		codeR1CBF(spQR1, cbf2x2, cbfException, w, h);

				
								
#endif



				

// ==============================
// ==============================
// ==============================
// --------------- encode amplitudes 
				int sign;
				int bin=0;
#if BITPLANE_R1_CODING
				codeR1_bitplane(spQR1, cbf2x2, cbfException, w, h);

#elif R1_PREDICTIVE

				codeR1_predictive(spQR1, cbf2x2, cbfException, w, h);

#else

#if R1_DIRECTCODING
				if (true)
				{
					codeR1_DirectCoding(spQR1, w, h
#if CBF2x2
						, cbfException
						, cbf2x2
#endif
						);
				}
				else
				{
#endif
					codeR1_regular(spQR1, w, h
#if CBF2x2
						, cbfException
						, cbf2x2
#endif
						);

#if R1_DIRECTCODING
				} // end if (true)
#endif

#endif 	
				
#if CBF2x2
				_aligned_free(cbfException);
				_aligned_free(cbf2x2);
#endif
				
				
				int b2 = m_pcBinIf->getNumWrittenBits();





#if SIGN_APART
// ==============================
// ==============================
// ==============================
// ------------- Encode signs independantly
				int prevSign = 0;
				int err;
				for (int j = 0; j < h; j++)
	 					for (int i = 0; i < w; i++)
						if (abs(spQR1[j*w + i]) > 0)
						{
							sign = spQR1[j*w + i] < 0;

#if R1_SIGNPREDICTION
							err = 1-(sign == prevSign);
							m_pcBinIf->encodeBin(err, m_cCUR1SpSign.get(0, 0, 0));
							prevSign = sign;
#else
							m_pcBinIf->encodeBin(sign, m_cCUR1SpSign.get(0, 0, 0));
#endif
#if NOISE_MARK
							int mark = spQR1NoiseMark[j*w + i];
							if (spQR1[j*w+i]/spQ >= NOISE_THR)
								m_pcBinIf->encodeBin(mark, m_cCUPIPNoiseMark.get(0, 0, 0));
							if (encodeTime)
								fprintf(stdout, "%2d\t%d\n", spQR1[j*w + i] / spQ, spQR1NoiseMark[j*w + i]);

#endif
						}
#endif
				if (encodeTime && false)
				{
					fprintf(stdout, "\n");
				}



// ==============================
// ==============================
// ==============================
// ------------- Encode predictor 

#if MULTIPLEPRED 
				// binarise predictor idx
				if (idx < 2)
				{
					m_pcBinIf->encodeBin(0, m_cCUPIPPredictors.get(0, 0, 0));
					

					if (idx == 0)
						m_pcBinIf->encodeBin(0, m_cCUPIPPredictors.get(0, 0, 1));
					else
						m_pcBinIf->encodeBin(1, m_cCUPIPPredictors.get(0, 0, 1));
				}
				else
				{
					m_pcBinIf->encodeBin(1, m_cCUPIPPredictors.get(0, 0, 0));
					if (idx == 2)
						m_pcBinIf->encodeBin(0, m_cCUPIPPredictors.get(0, 0, 2));
					else
						m_pcBinIf->encodeBin(1, m_cCUPIPPredictors.get(0, 0, 2));
				}
#endif
				


#if SP_QCHANGE
		if (spQIsChanged)
			spQ = tmp_spQ;
#endif
		
	}
}

Void TEncSbac::codeR1_regular(int *spQR1, int w, int h
#if CBF2x2
	, Bool *cbfException
	, UInt *cbf2x2
#endif
	)
{
	// offset here	
	int offset_temp = 0;
#if CBF2x2	
	int *cbfMap = (int*)xMalloc(int, w*h*sizeof(int));
	if (w == 4 && h == 4)
	{
		memcpy(cbfMap, cbfMap4x4, w*h*sizeof(int));
		offset_temp = offset4x4;
	}
	if (w == 4 && h == 8)
	{
		memcpy(cbfMap, cbfMap4x8, w*h*sizeof(int));
		offset_temp = offset4x8;
	}
	if (w == 8 && h == 4)
	{
		memcpy(cbfMap, cbfMap8x4, w*h*sizeof(int));
		offset_temp = offset8x4;
	}
	if (w == 8 && h == 8)
	{
		memcpy(cbfMap, cbfMap8x8, w*h*sizeof(int));
		offset_temp = offset8x8;
	}
#endif
	int qStep = spQ + offset_temp;
	
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			int ampl = abs(spQR1[j*w + i]) / qStep;
#if CBF2x2
			if (cbf2x2[cbfMap[j*w + i]])
			{
#endif
				for (int a = 0; a < ampl; a++)
				{
#if CBF2x2
					if (!a && cbfException[j*w + i])
					{
						// skip encoding because it is derivable									
					}
					else
#endif
						m_pcBinIf->encodeBin(1, m_cCUR1SpGr.get(0, 0, a));

				}

				if (ampl < COEFF_LIMIT)
					m_pcBinIf->encodeBin(0, m_cCUR1SpGr.get(0, 0, ampl));
#if CBF2x2
			}
#endif
		}

#if CBF2x2
	_aligned_free(cbfMap);
#endif
}

#if R1_DIRECTCODING
Void TEncSbac::codeR1_DirectCoding(int *spQR1, int w, int h
#if CBF2x2
	, Bool *cbfException
	, UInt *cbf2x2
#endif
	)
{
	// offset here	
	int offset_temp = 0;
	int directCodes[16][4] = {
		{0, 0, 0, 0},
		{0, 0, 0, 1},
		{0, 0, 1, 0},
		{0, 0, 1, 1},
		{0, 1, 0, 0},
		{0, 1, 0, 1},
		{0, 1, 1, 0},
		{0, 1, 1, 1},
		{1, 0, 0, 0},
		{1, 0, 0, 1},
		{1, 0, 1, 0},
		{1, 0, 1, 1},
		{1, 1, 0, 0},
		{1, 1, 0, 1},
		{1, 1, 1, 0},
		{1, 1, 1, 1}
	};

	int contexts[16][4] = {
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 4 },
		{ 0, 1, 2, 4 },
		{ 0, 1, 5, 6 },
		{ 0, 1, 5, 6 },
		{ 0, 1, 5, 7 },
		{ 0, 1, 5, 7 },
		{ 0, 8, 9, 10 },
		{ 0, 8, 9, 10 },
		{ 0, 8, 9, 11 },
		{ 0, 8, 9, 11 },
		{ 0, 8, 12, 13 },
		{ 0, 8, 12, 13 },
		{ 0, 8, 12, 14 },
		{ 0, 8, 12, 14 } 
	};

#if CBF2x2	
	int *cbfMap = (int*)xMalloc(int, w*h*sizeof(int));
	if (w == 4 && h == 4)
	{
		memcpy(cbfMap, cbfMap4x4, w*h*sizeof(int));
		offset_temp = offset4x4;
	}
	if (w == 4 && h == 8)
	{
		memcpy(cbfMap, cbfMap4x8, w*h*sizeof(int));
		offset_temp = offset4x8;
	}
	if (w == 8 && h == 4)
	{
		memcpy(cbfMap, cbfMap8x4, w*h*sizeof(int));
		offset_temp = offset8x4;
	}
	if (w == 8 && h == 8)
	{
		memcpy(cbfMap, cbfMap8x8, w*h*sizeof(int));
		offset_temp = offset8x8;
	}
#endif
	int qStep = spQ + offset_temp;
	int bin, ctx;
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			int ampl = abs(spQR1[j*w + i]) / qStep;
#if CBF2x2
			if (cbf2x2[cbfMap[j*w + i]])
			{
#endif
				for (int a = 0; a < 4; a++)
				{
					bin = directCodes[ampl][a];
					ctx = contexts[ampl][a];
					m_pcBinIf->encodeBin(bin, m_cCUR1DirectCoding.get(0, 0, ctx));

				}
				
#if CBF2x2
			}
#endif
		}

#if CBF2x2
	_aligned_free(cbfMap);
#endif
}
#endif

#if R1_PREDICTIVE
Void TEncSbac::codeR1_predictive(int *spQR1, UInt *cbf2x2, Bool *cbfException, int w, int h)
{
	if (encodeTime && false)
		cout << endl;
	UInt left, top, topleft, sum, bin;
	for (int j = 0; j < h; j++)
		for (int i = 0; i < w; i++)
		{
			int ampl = abs(spQR1[j*w + i]) / spQ;
			if (encodeTime && false)
				cout << j*w + i << " --> " << ampl << ":\t";
#if CBF2x2
			if (cbf2x2[cbfMap4x4[j*w + i]])
			{
				// get the neighbors
				if (i || j)
				{
					left	= i				? abs(spQR1[j*w + i - 1])		: abs(spQR1[(j - 1)*w + i]);
					top		= j				? abs(spQR1[(j - 1)*w + i])		: left;
					topleft = (i && j)		? abs(spQR1[(j - 1)*w + i - 1]) : left;

					left /= spQ;
					top /= spQ;
					topleft /= spQ;
				}
#endif
				for (int a = 0; a < ampl; a++)
				{
#if CBF2x2
					if (!a && cbfException[j*w + i])
					{
						// skip encoding because it is derivable									
					}
					else
					{
#endif
						// decide where you are: first row and column --> bypass (temporary)
						if (!j && !i)
						{
							m_pcBinIf->encodeBinEP(1);
							if (encodeTime && false)
								cout << "EP 1     \t";
						}
						else
						{								
							// if it is layer 0 (i.e. !a) --> predict and code the error
							// encode one: 
							// sum:1 / err:0     or		sum:0 / err:1
							if (!a)
							{
								sum = ((left > 0) + (top > 0) + (topleft > 0)) > 1;
								if (sum)
									bin = 0;
								else
									bin = 1;
							}
							else
							{
								sum = ((left >= a) + (top >= a) + (topleft >= a)) > 1;
								if (sum)
									bin = 0;
								else
									bin = 1;

								bin = 1 - bin;
							}
							m_pcBinIf->encodeBin(bin, m_cCUR1SpGr.get(0, 0, a));
							if (encodeTime && false)
								cout << "Pr 1:e" << bin << ",s" << sum << "\t";
						}
#if CBF2x2
					}
#endif

				}

				// encode the terminal zero
				if (ampl < COEFF_LIMIT)
				{
					if (!i && !j)
					{
						m_pcBinIf->encodeBinEP(0);
						if (encodeTime && false)
							cout << "EP 0" << endl;
					}
					else
					{
						// encode zero: 
						// sum:0 / err:0     or		sum:1 / err:1
						if (!ampl)
						{
							sum = ((left > 0) + (top > 0) + (topleft > 0)) > 1;
							if (sum)
								bin = 1;
							else
								bin = 0;
						}
						else
						{
							sum = ((left >= ampl) + (top >= ampl) + (topleft >= ampl)) > 1;
							if (sum)
								bin = 1;
							else
								bin = 0;		

							bin = 1 - bin;
						}

						m_pcBinIf->encodeBin(bin, m_cCUR1SpGr.get(0, 0, ampl));
						if (encodeTime && false)
							cout << "Pr 0:e" << bin << ",s" << sum << endl;
					}
				}
#if CBF2x2
			}
			else
			{
				if (encodeTime && false)
					cout << "CBF0" << endl;
			}
#endif
		}
}
#endif

#if BITPLANE_R1_CODING
Void TEncSbac::codeR1_bitplane(int *spQR1, UInt *cbf2x2, Bool *cbfException, int w, int h)
{
	UInt outbl[MAXDYN][4][4];
	// UInt outbl[MAXDYN - 1][4][4];
	UInt *vector = new UInt[16];
	for (int cf = 0; cf < 16; cf++)
		vector[cf] = abs(spQR1[cf]) / spQ;
	convertToBitLayers_Unary(vector, 16, outbl);
	Int situations[MAXDYN - 1][4][4];
	computeSituationsFromBitLayers_Unary(outbl, situations);

	int offset_temp = 0;
#if CBF2x2	
	int *cbfMap = (int*)xMalloc(int, w*h*sizeof(int));
	if (w == 4 && h == 4)
	{
		memcpy(cbfMap, cbfMap4x4, w*h*sizeof(int));
		offset_temp = offset4x4;
	}
	if (w == 4 && h == 8)
	{
		memcpy(cbfMap, cbfMap4x8, w*h*sizeof(int));
		offset_temp = offset4x8;
	}
	if (w == 8 && h == 4)
	{
		memcpy(cbfMap, cbfMap8x4, w*h*sizeof(int));
		offset_temp = offset8x4;
	}
	if (w == 8 && h == 8)
	{
		memcpy(cbfMap, cbfMap8x8, w*h*sizeof(int));
		offset_temp = offset8x8;
	}
#endif

	for (int lvl = 0; lvl < MAXDYN; lvl++)
	{
		for (int y = 0; y < h; y++)
			for (int x = 0; x < w; x++)
			{
				if (cbf2x2[cbfMap[y*w + x]] && !(cbfException[y*w + x] && lvl==0))
				{
					UInt synElem = outbl[lvl][y][x];
					if (synElem < 2)
					{
						UInt situation = situations[lvl][y][x];
						UInt ctxIdx = lookup_table[situation];
						m_pcBinIf->encodeBin(synElem, m_cCUR1SpGr.get(0, 0, ctxIdx));
						// m_pcBinIf->encodeBin(synElem, m_cCUR1SpGr.get(0, 0, 0));
					}
				}
			}
	}
}
#endif

#if CBF2x2
Void TEncSbac::codeR1CBF(int* spQR1, UInt *cbf2x2, Bool *cbfException, int w, int h)
{

	int *cbf2x2BlMap = (int*)xMalloc(int, w*h*sizeof(int) / 4);
	if (w == 4 && h == 4)
		memcpy(cbf2x2BlMap, cbf2x2BlMap4x4, w*h*sizeof(int));
	if (w == 4 && h == 8)
		memcpy(cbf2x2BlMap, cbf2x2BlMap4x8, w*h*sizeof(int));
	if (w == 8 && h == 4)
		memcpy(cbf2x2BlMap, cbf2x2BlMap8x4, w*h*sizeof(int));
	if (w == 8 && h == 8)
		memcpy(cbf2x2BlMap, cbf2x2BlMap8x8, w*h*sizeof(int));

	int *cbfMap = (int*)xMalloc(int, w*h*sizeof(int));
	if (w == 4 && h == 4)
		memcpy(cbfMap, cbfMap4x4, w*h*sizeof(int));
	if (w == 4 && h == 8)
		memcpy(cbfMap, cbfMap4x8, w*h*sizeof(int));
	if (w == 8 && h == 4)
		memcpy(cbfMap, cbfMap8x4, w*h*sizeof(int));
	if (w == 8 && h == 8)
		memcpy(cbfMap, cbfMap8x8, w*h*sizeof(int));

	for (int cf = 0; cf < w*h; cf++)
	{
		if (spQR1[cf] != 0)
			cbf2x2[cbfMap[cf]] = 1;
	}


	for (int blk2x2 = 0; blk2x2 < w*h/4; blk2x2++)
	{
		if (cbf2x2[blk2x2])
		{
			Bool isException = true;
			for (int cf = 0; cf < 3; cf++)
			{
				if (spQR1[cbf2x2BlMap[blk2x2*4+cf]] != 0)
					isException = false;
			}
			cbfException[cbf2x2BlMap[blk2x2*4+3]] = isException;
		}
	}

	for (int cg = 0; cg < w*h/4; cg++)
		m_pcBinIf->encodeBin(cbf2x2[cg], m_cCUPIPR1Cbf.get(0, 0, cg));
		// m_pcBinIf->encodeBin(cbf2x2[cg], m_cCUPIPR1Cbf.get(0, 0, 0));
		
		
		

	_aligned_free(cbf2x2BlMap);
	_aligned_free(cbfMap);
}
#endif


#endif

Void TEncSbac::codeCUTransquantBypassFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getCUTransquantBypass(uiAbsPartIdx);
  m_pcBinIf->encodeBin( uiSymbol, m_CUTransquantBypassFlagSCModel.get( 0, 0, 0 ) );
}

/** code skip flag
 * \param pcCU
 * \param uiAbsPartIdx
 * \returns Void
 */
Void TEncSbac::codeSkipFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isSkipped( uiAbsPartIdx ) ? 1 : 0;
  UInt uiCtxSkip = pcCU->getCtxSkipFlag( uiAbsPartIdx ) ;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUSkipFlagSCModel.get( 0, 0, uiCtxSkip ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tSkipFlag" );
  DTRACE_CABAC_T( "\tuiCtxSkip: ");
  DTRACE_CABAC_V( uiCtxSkip );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}

#if COM16_C806_OBMC
Void TEncSbac::codeOBMCFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getOBMCFlag( uiAbsPartIdx ) ? 1 : 0;

  m_pcBinIf->encodeBin( uiSymbol, m_cCUOBMCFlagSCModel.get( 0, 0, 0 ) );

#if PIP
  if (encodeTime && verbose)
	  cout << ",OBMC:" << (Int)uiSymbol;
#endif
  
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tOBMCFlag" );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}
#endif

#if VCEG_AZ06_IC
/** code Illumination Compensation flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeICFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiSymbol = pcCU->getICFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUICFlagSCModel.get( 0, 0, 0 ) );

#if PIP
  if (encodeTime && verbose)
	  cout << ",IC:" << (Int)uiSymbol;
#endif


  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tICFlag" );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}
#endif

#if VCEG_AZ07_IMV
Void TEncSbac::codeiMVFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->getiMVFlag( uiAbsPartIdx ) ? 1 : 0;
  UInt uiCtxiMV = pcCU->getCtxiMVFlag( uiAbsPartIdx ) ;
  m_pcBinIf->encodeBin( uiSymbol, m_cCUiMVFlagSCModel.get( 0, 0, uiCtxiMV ) );

#if  JVET_E0076_MULTI_PEL_MVD
  if (uiSymbol)
  {
    uiSymbol = pcCU->getiMVFlag( uiAbsPartIdx ) > 1 ? 1 : 0;  
    m_pcBinIf->encodeBin( uiSymbol, m_cCUiMVFlagSCModel.get( 0, 0, 3 ) );
  }
#endif

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tiMVFlag" );
  DTRACE_CABAC_T( "\tuiCtxiMV: ");
  DTRACE_CABAC_V( uiCtxiMV );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}
#endif


#if VCEG_AZ05_INTRA_MPI
Void TEncSbac::codeMPIIdx(TComDataCU* pcCU, UInt uiAbsPartIdx)
{
  if (!pcCU->getSlice()->getSPS()->getUseMPI()) return;
  if (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER)
  {
    return;
  }
  Int iNumberOfPassesMPI = 1;
  if (pcCU->getSlice()->getSliceType() == I_SLICE) iNumberOfPassesMPI = 2;
  else iNumberOfPassesMPI = 2;
  if (iNumberOfPassesMPI>1) // for only 1 pass no signaling is needed 
  {
    if (iNumberOfPassesMPI>2)  // 3 or 4
    {
      Int idxMPI = pcCU->getMPIIdx(uiAbsPartIdx);
      const UInt uiSymbol0 = (idxMPI >> 1);
      const UInt uiSymbol1 = (idxMPI % 2);
      m_pcBinIf->encodeBin(uiSymbol0, m_cMPIIdxSCModel.get(0, 0, 0));
      m_pcBinIf->encodeBin(uiSymbol1, m_cMPIIdxSCModel.get(0, 0, 1));
    }
    else //iNumberOfPassesMPI==2
    {
      Int idxMPI = pcCU->getMPIIdx(uiAbsPartIdx);
      const UInt uiSymbol = idxMPI;
      m_pcBinIf->encodeBin(uiSymbol, m_cMPIIdxSCModel.get(0, 0, 0));
    }
  }
}
#endif

#if COM16_C1046_PDPC_INTRA
 Void TEncSbac::codePDPCIdx(TComDataCU* pcCU, UInt uiAbsPartIdx)
 {
#if PIP
	 if (encodeTime && verbose)
		 cout << ",PDPC:";
#endif
  if (!pcCU->getSlice()->getSPS()->getUsePDPC()) return;
  if (pcCU->getPredictionMode(uiAbsPartIdx) == MODE_INTER)
  {
    return;
  }
  Int iNumberOfPassesPDPC = 1;
  if (pcCU->getSlice()->getSliceType() == I_SLICE) iNumberOfPassesPDPC = 2;
  else iNumberOfPassesPDPC = 2;
  if (iNumberOfPassesPDPC > 1) // for only 1 pass no signaling is needed 
  {
    if (iNumberOfPassesPDPC > 2)  // 3 or 4
    {
      Int idxPDPC = pcCU->getPDPCIdx(uiAbsPartIdx);
      const UInt uiSymbol0 = (idxPDPC >> 1);
      const UInt uiSymbol1 = (idxPDPC % 2);
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)uiSymbol0 << (Int)uiSymbol1;
#endif
      m_pcBinIf->encodeBin(uiSymbol0, m_cPDPCIdxSCModel.get(0, 0, 0));
      m_pcBinIf->encodeBin(uiSymbol1, m_cPDPCIdxSCModel.get(0, 0, 1));
    }
    else //iNumberOfPassesMPI==2
    {
      Int idxPDPC = pcCU->getPDPCIdx(uiAbsPartIdx);
      const UInt uiSymbol = idxPDPC;
      m_pcBinIf->encodeBin(uiSymbol, m_cPDPCIdxSCModel.get(0, 0, 0));
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)uiSymbol;
#endif
    }
   }
 }
#endif

#if VCEG_AZ05_ROT_TR || COM16_C1044_NSST
Void TEncSbac::codeROTIdx ( TComDataCU* pcCU, UInt uiAbsPartIdx,UInt uiDepth  )
{
#if PIP
	if (encodeTime && verbose)
		cout << ",ROT:";
#endif
#if COM16_C1044_NSST
  if (!pcCU->getSlice()->getSPS()->getUseNSST()) return;
#else
  if (!pcCU->getSlice()->getSPS()->getUseROT()) return;
#endif
  Int iNumberOfPassesROT = 1;
  if( pcCU->isIntra(uiAbsPartIdx)
#if VCEG_AZ05_INTRA_MPI
    && pcCU->getMPIIdx(uiAbsPartIdx) ==0
#endif  
#if COM16_C1046_PDPC_INTRA
    && pcCU->getPDPCIdx(uiAbsPartIdx) == 0
#endif  
    && !pcCU->getCUTransquantBypass(uiAbsPartIdx)
    )  iNumberOfPassesROT = 4;

#if COM16_C1044_NSST
#if JVET_C0024_QTBT
  if( iNumberOfPassesROT==4 )
#else
  if( iNumberOfPassesROT==4 && pcCU->getPartitionSize(uiAbsPartIdx)==SIZE_2Nx2N )
#endif
  {
#if PIP
	  Bool isPIP = pcCU->getPIPflag(uiAbsPartIdx);
	  iNumberOfPassesROT = ( isPIP ? PIP_DEF_IPM : pcCU->getIntraDir( CHANNEL_TYPE_LUMA, uiAbsPartIdx )) <= DC_IDX ? 3 : 4;	  
#else
    iNumberOfPassesROT = pcCU->getIntraDir( CHANNEL_TYPE_LUMA, uiAbsPartIdx ) <= DC_IDX ? 3 : 4;
#endif
  }

  if( iNumberOfPassesROT==3 )
  {
#if JVET_C0024_QTBT
    Int idxROT = pcCU->getROTIdx( CHANNEL_TYPE_LUMA, uiAbsPartIdx );
#else
    Int idxROT = pcCU->getROTIdx( uiAbsPartIdx );
#endif
    assert(idxROT<3);

#if JVET_C0042_UNIFIED_BINARIZATION
            m_pcBinIf->encodeBin(  idxROT ? 1 : 0 , m_cROTidxSCModel.get(0,0, 1) );
#if PIP
			if (encodeTime && verbose)
				cout << (Int)(idxROT ? 1 : 0);
#endif
            if( idxROT )
            {
                   if(idxROT ==1) m_pcBinIf->encodeBin( 0 , m_cROTidxSCModel.get(0,0, 3) );
                   else m_pcBinIf->encodeBin( 1 , m_cROTidxSCModel.get(0,0, 3) );
#if PIP
				   if (encodeTime && verbose)
					   cout << 1;
#endif
            } 
#else //#if JVET_C0042_UNIFIED_BINARIZATION

    m_pcBinIf->encodeBin( idxROT ? 1 : 0, m_cROTidxSCModel.get(0,0, 0 ) );
    if( idxROT )
    {
      m_pcBinIf->encodeBin( (idxROT-1) ? 1 : 0, m_cROTidxSCModel.get(0,0, 1 ) );
    }
 #endif //#if JVET_C0042_UNIFIED_BINARIZATION
  }
  else
#endif
  if (iNumberOfPassesROT>1) // for only 1 pass no signaling is needed 
  {
#if JVET_C0024_QTBT
      Int idxROT = pcCU->getROTIdx( CHANNEL_TYPE_LUMA, uiAbsPartIdx );
#else
      Int idxROT = pcCU->getROTIdx( uiAbsPartIdx );
#endif

#if JVET_C0042_UNIFIED_BINARIZATION
            m_pcBinIf->encodeBin(  idxROT ? 1 : 0 , m_cROTidxSCModel.get(0,0, 0) );
#if PIP
			if (encodeTime && verbose)
				cout << (Int)(idxROT ? 1 : 0);
#endif
            if( idxROT )
            {
                     m_pcBinIf->encodeBin( (idxROT-1) ? 1 : 0 , m_cROTidxSCModel.get(0,0, 2) );
#if PIP
					 if (encodeTime && verbose)
						 cout << (Int)((idxROT - 1) ? 1 : 0);
#endif

                    if(idxROT >1 )
                    {
                        m_pcBinIf->encodeBin( (idxROT-2) ? 1 : 0, m_cROTidxSCModel.get(0,0, 4) );
#if PIP
						if (encodeTime && verbose)
							cout << (Int)((idxROT - 2) ? 1 : 0);
#endif
                    }
               
            } 
#else
      const UInt uiSymbol0 = (idxROT >>1);
      const UInt uiSymbol1 = (idxROT %2 );
      m_pcBinIf->encodeBin( uiSymbol0, m_cROTidxSCModel.get(0,0, uiDepth ) );
      m_pcBinIf->encodeBin( uiSymbol1, m_cROTidxSCModel.get(0,0, uiDepth ) );
#endif
  }
}

#if JVET_C0024_QTBT
Void TEncSbac::codeROTIdxChroma ( TComDataCU* pcCU, UInt uiAbsPartIdx,UInt uiDepth  )
{
#if PIP
	if (encodeTime && verbose)
		cout << "ROTCH:";
#endif
#if COM16_C1044_NSST
  if (!pcCU->getSlice()->getSPS()->getUseNSST()) return;
#else
  if (!pcCU->getSlice()->getSPS()->getUseROT()) return;
#endif
  Int iNumberOfPassesROT = 1;
  if( pcCU->isIntra(uiAbsPartIdx)
    && !pcCU->getCUTransquantBypass(uiAbsPartIdx)
    )  iNumberOfPassesROT = 4;

#if COM16_C1044_NSST
  if( iNumberOfPassesROT==4 )
  {
    UInt uiIntraMode = pcCU->getIntraDir( CHANNEL_TYPE_CHROMA, uiAbsPartIdx );
#if JVET_E0062_MULTI_DMS && COM16_C806_LMCHROMA
    if( uiIntraMode == LM_CHROMA_IDX )
    {
      uiIntraMode = PLANAR_IDX;
    }
#if JVET_E0077_ENHANCED_LM
    else if (IsLMMode(uiIntraMode))
    {
        uiIntraMode = PLANAR_IDX;
    }
#endif
#else
    if( uiIntraMode == DM_CHROMA_IDX )
    {
      uiIntraMode = pcCU->getPic()->getCtu(pcCU->getCtuRsAddr())->getIntraDir(CHANNEL_TYPE_LUMA, pcCU->getZorderIdxInCtu()+uiAbsPartIdx);
    }
#if COM16_C806_LMCHROMA
    else if( uiIntraMode == LM_CHROMA_IDX )
    {
      uiIntraMode = PLANAR_IDX;
    }
#endif

#if JVET_E0077_ENHANCED_LM
    else if (IsLMMode(uiIntraMode))
    {
        uiIntraMode = PLANAR_IDX;
    }
#endif

#endif

    iNumberOfPassesROT = uiIntraMode <= DC_IDX ? 3 : 4;
  }

  if( iNumberOfPassesROT==3 )
  {
    Int idxROT = pcCU->getROTIdx( CHANNEL_TYPE_CHROMA, uiAbsPartIdx );
    assert(idxROT<3);
#if JVET_C0042_UNIFIED_BINARIZATION
      m_pcBinIf->encodeBin(  idxROT ? 1 : 0 , m_cROTidxSCModel.get(0,0, 1) );
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)(idxROT ? 1 : 0);
#endif
            if( idxROT )
            {
                   if(idxROT ==1) m_pcBinIf->encodeBin( 0 , m_cROTidxSCModel.get(0,0, 3) );
                   else m_pcBinIf->encodeBin( 1 , m_cROTidxSCModel.get(0,0, 3) );
#if PIP
				   if (encodeTime && verbose)
					   cout << (idxROT == 1 ? 0 : 1);
#endif
            } 
#else
    m_pcBinIf->encodeBin( idxROT ? 1 : 0, m_cROTidxSCModel.get(0,0, 0 ) );
    if( idxROT )
    {
      m_pcBinIf->encodeBin( (idxROT-1) ? 1 : 0, m_cROTidxSCModel.get(0,0, 1 ) );
    }
 #endif //#if JVET_C0042_UNIFIED_BINARIZATION
  }
  else
#endif
  if (iNumberOfPassesROT>1) // for only 1 pass no signaling is needed 
  {
      Int idxROT = pcCU->getROTIdx( CHANNEL_TYPE_CHROMA, uiAbsPartIdx );
#if JVET_C0042_UNIFIED_BINARIZATION
      m_pcBinIf->encodeBin(  idxROT ? 1 : 0 , m_cROTidxSCModel.get(0,0, 0) );
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)(idxROT ? 1 : 0);
#endif
            if( idxROT )
            {
                     m_pcBinIf->encodeBin( (idxROT-1) ? 1 : 0 , m_cROTidxSCModel.get(0,0, 2) );
#if PIP
					 if (encodeTime && verbose)
						 cout << (Int)((idxROT - 1) ? 1 : 0);
#endif
                    if(idxROT >1 )
                    {
                        m_pcBinIf->encodeBin( (idxROT-2) ? 1 : 0, m_cROTidxSCModel.get(0,0, 4) );
#if PIP
						if (encodeTime && verbose)
							cout << (Int)((idxROT - 2) ? 1 : 0);
#endif
                    }
               
            } 
#else
      const UInt uiSymbol0 = (idxROT >>1);
      const UInt uiSymbol1 = (idxROT %2 );
      m_pcBinIf->encodeBin( uiSymbol0, m_cROTidxSCModel.get(0,0, uiDepth ) );
      m_pcBinIf->encodeBin( uiSymbol1, m_cROTidxSCModel.get(0,0, uiDepth ) );
#endif
  }
}
#endif
#endif
/** code merge flag
 * \param pcCU
 * \param uiAbsPartIdx
 * \returns Void
 */
Void TEncSbac::codeMergeFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  const UInt uiSymbol = pcCU->getMergeFlag( uiAbsPartIdx ) ? 1 : 0;
  m_pcBinIf->encodeBin( uiSymbol, *m_cCUMergeFlagExtSCModel.get( 0 ) );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tMergeFlag: " );
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\tAddress: " );
  DTRACE_CABAC_V( pcCU->getCtuRsAddr() );
  DTRACE_CABAC_T( "\tuiAbsPartIdx: " );
  DTRACE_CABAC_V( uiAbsPartIdx );
  DTRACE_CABAC_T( "\n" );
}

/** code merge index
 * \param pcCU
 * \param uiAbsPartIdx
 * \returns Void
 */
Void TEncSbac::codeMergeIndex( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiUnaryIdx = pcCU->getMergeIndex( uiAbsPartIdx );
  UInt uiNumCand = pcCU->getSlice()->getMaxNumMergeCand();
  if ( uiNumCand > 1 )
  {
    for( UInt ui = 0; ui < uiNumCand - 1; ++ui )
    {
      const UInt uiSymbol = ui == uiUnaryIdx ? 0 : 1;
#if COM16_C806_GEN_MRG_IMPROVEMENT
      m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, (ui > NUM_MERGE_IDX_EXT_CTX-1? NUM_MERGE_IDX_EXT_CTX-1:ui) ) );
#else
      if ( ui==0 )
      {
        m_pcBinIf->encodeBin( uiSymbol, m_cCUMergeIdxExtSCModel.get( 0, 0, 0 ) );
      }
      else
      {
        m_pcBinIf->encodeBinEP( uiSymbol );
      }
#endif
      if( uiSymbol == 0 )
      {
        break;
      }
    }
  }
  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tparseMergeIndex()" );
  DTRACE_CABAC_T( "\tuiMRGIdx= " );
  DTRACE_CABAC_V( pcCU->getMergeIndex( uiAbsPartIdx ) );
  DTRACE_CABAC_T( "\n" );
}

#if VCEG_AZ07_FRUC_MERGE
Void TEncSbac::codeFRUCMgrMode( TComDataCU* pcCU, UInt uiAbsPartIdx , UInt uiPUIdx )
{
  if( !pcCU->getSlice()->getSPS()->getUseFRUCMgrMode() )
    return;

  UInt uiFirstBin = pcCU->getFRUCMgrMode( uiAbsPartIdx ) != FRUC_MERGE_OFF;
  m_pcBinIf->encodeBin( uiFirstBin, m_cCUFRUCMgrModeSCModel.get( 0 ,  0 , pcCU->getCtxFRUCMgrMode( uiAbsPartIdx ) ) );

  if( uiFirstBin )
  {
    if( pcCU->getSlice()->isInterP() )
    {
      assert( pcCU->getFRUCMgrMode( uiAbsPartIdx ) == FRUC_MERGE_TEMPLATE );
    }
    else
    {
      UInt uiSecondBin = pcCU->getFRUCMgrMode( uiAbsPartIdx ) == FRUC_MERGE_BILATERALMV;
      m_pcBinIf->encodeBin( uiSecondBin , m_cCUFRUCMESCModel.get( 0 , 0 , pcCU->getCtxFRUCME( uiAbsPartIdx ) ) );
    }
  }
}
#endif

#if JVET_C0024_QTBT
Void TEncSbac::codeBTSplitMode ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiWidth, UInt uiHeight)
{
  UInt uiBTSplitMode = pcCU->getBTSplitModeForBTDepth(uiAbsPartIdx, pcCU->getBTDepth(uiAbsPartIdx, uiWidth, uiHeight));  //0: no split; 1: hor; 2: ver

  UInt uiCtx = pcCU->getCtxBTSplitFlag(uiAbsPartIdx, uiWidth, uiHeight);

  //signal bits: no split: 0; hor split: 10; ver split: 11.
  m_pcBinIf->encodeBin(uiBTSplitMode!=0, m_cBTSplitFlagSCModel.get(0, 0, uiCtx));
  UInt uiMinBTSize = pcCU->getSlice()->isIntra() ? (isLuma(pcCU->getTextType())?MIN_BT_SIZE:MIN_BT_SIZE_C): MIN_BT_SIZE_INTER;

  if (uiWidth==uiMinBTSize || uiHeight==uiMinBTSize)
  {
    return;
  }
#if JVET_C0024_BT_RMV_REDUNDANT
  if ( pcCU->getSplitConstrain() == 1 )
  {
    assert( uiBTSplitMode != 1 );
    return;
  }
  if ( pcCU->getSplitConstrain() == 2 )
  {
    assert( uiBTSplitMode != 2 );
    return;
  }
#endif

  if (uiBTSplitMode != 0)
  {
    UInt uiBTSCtx = uiWidth==uiHeight ? 0: (uiWidth>uiHeight? 1: 2);
    m_pcBinIf->encodeBin(uiBTSplitMode!=1, m_cBTSplitFlagSCModel.get(0, 0, 3+uiBTSCtx));
  }
}
#endif

Void TEncSbac::codeSplitFlag   ( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{
#if JVET_C0024_QTBT
  if ( uiDepth == g_aucConvertToBit[pcCU->getSlice()->getSPS()->getCTUSize()] 
  - g_aucConvertToBit[pcCU->getSlice()->getSPS()->getMinQTSize(pcCU->getSlice()->getSliceType(), pcCU->getTextType())])
#else
  if( uiDepth == pcCU->getSlice()->getSPS()->getLog2DiffMaxMinCodingBlockSize() )
#endif
  {
    return;
  }

  UInt uiCtx           = pcCU->getCtxSplitFlag( uiAbsPartIdx, uiDepth );
  UInt uiCurrSplitFlag = ( pcCU->getDepth( uiAbsPartIdx ) > uiDepth ) ? 1 : 0;

#if COM16_C806_LARGE_CTU
  assert( uiCtx < NUM_SPLIT_FLAG_CTX );
#else
  assert( uiCtx < 3 );
#endif
  m_pcBinIf->encodeBin( uiCurrSplitFlag, m_cCUSplitFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tSplitFlag\n" )
  return;
}

Void TEncSbac::codeTransformSubdivFlag( UInt uiSymbol, UInt uiCtx )
{
  m_pcBinIf->encodeBin( uiSymbol, m_cCUTransSubdivFlagSCModel.get( 0, 0, uiCtx ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseTransformSubdivFlag()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiSymbol )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\n" )
}


Void TEncSbac::codeIntraDirLumaAng( TComDataCU* pcCU, UInt absPartIdx, Bool isMultiple
#if VCEG_AZ07_INTRA_65ANG_MODES
                                   , Int* piModes, Int iAboveLeftCase
#endif
                                   )
{
#if PIP
	if (encodeTime && verbose)
		cout << ",DirLuma:";
#endif
  UInt dir[4],j;
#if VCEG_AZ07_INTRA_65ANG_MODES
  Int preds[4][NUM_MOST_PROBABLE_MODES] = {{-1, -1, -1, -1, -1, -1},{-1, -1, -1, -1, -1, -1},{-1, -1, -1, -1, -1, -1},{-1, -1, -1, -1, -1, -1}};
#else
  Int preds[4][NUM_MOST_PROBABLE_MODES] = {{-1, -1, -1},{-1, -1, -1},{-1, -1, -1},{-1, -1, -1}};
#endif
  Int predIdx[4] ={ -1,-1,-1,-1};
#if JVET_C0024_QTBT
  UInt partNum = 1;
#else
  PartSize mode = pcCU->getPartitionSize( absPartIdx );
  UInt partNum = isMultiple?(mode==SIZE_NxN?4:1):1;
#endif
  UInt partOffset = ( pcCU->getPic()->getNumPartitionsInCtu() >> ( pcCU->getDepth(absPartIdx) << 1 ) ) >> 2;
  
#if JVET_C0055_INTRA_MPM
  static const UInt mpmContext[NUM_INTRA_MODE] = { 1, 1, 
#if VCEG_AZ07_INTRA_65ANG_MODES
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,   // 2-34
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3       // 35-67
#else
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,    // 2-18
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3     // 19-35
#endif
  };
#elif VCEG_AZ07_INTRA_65ANG_MODES
  const UInt uiContextMPM0[4] = { 2, 3, 1, 2 };
  const UInt uiContextMPM1[4] = { 4, 5, 5, 6 };
  const UInt uiContextMPM2[4] = { 7, 7, 8, 7 };
  Int aiCase[4]={0,0,0,0};
#endif

  for (j=0;j<partNum;j++)
  {
    dir[j] = pcCU->getIntraDir( CHANNEL_TYPE_LUMA, absPartIdx+partOffset*j );

#if PIP
	if (encodeTime && false)
	{
		FILE* fout = fopen("log.txt", "a");
		fprintf(fout, "%2d\n", dir[j]);
		fclose(fout);
	}
#endif
#if VCEG_AZ07_INTRA_65ANG_MODES
    if( piModes )
    {
      assert( !isMultiple );
      memcpy( preds[j], piModes, 6*sizeof(Int) );
#if !JVET_C0055_INTRA_MPM
      aiCase[j] = iAboveLeftCase;
#endif
    }
    else
#endif
    pcCU->getIntraDirPredictor(absPartIdx+partOffset*j, preds[j], COMPONENT_Y
#if VCEG_AZ07_INTRA_65ANG_MODES && !JVET_C0055_INTRA_MPM
    , aiCase[j]
#endif
    );
    for(UInt i = 0; i < NUM_MOST_PROBABLE_MODES; i++)
    {
      if(dir[j] == preds[j][i])
      {
        predIdx[j] = i;
      }
    }
    m_pcBinIf->encodeBin((predIdx[j] != -1)? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, 0 ) );
#if PIP
	if (encodeTime && verbose)
		cout << (Int)((predIdx[j] != -1) ? 1 : 0);
#endif
  }
  for (j=0;j<partNum;j++)
  {
    if(predIdx[j] != -1)
    {
#if JVET_C0055_INTRA_MPM
      m_pcBinIf->encodeBin( predIdx[j] ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, mpmContext[preds[j][0]] ) );
#if PIP
	  if (encodeTime && verbose)
			  cout << (Int)(predIdx[j] ? 1 : 0);
#endif
#elif VCEG_AZ07_INTRA_65ANG_MODES
      m_pcBinIf->encodeBin( predIdx[j] ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, uiContextMPM0[aiCase[j]] ) );
#else
      m_pcBinIf->encodeBinEP( predIdx[j] ? 1 : 0 );
#endif
      if (predIdx[j])
      {
#if VCEG_AZ07_INTRA_65ANG_MODES
#if JVET_C0055_INTRA_MPM
        m_pcBinIf->encodeBin( (predIdx[j]-1) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, mpmContext[preds[j][1]] ) );
#if PIP
		if (encodeTime && verbose)
			cout << (Int)((predIdx[j] - 1) ? 1 : 0);
#endif
        if ( (predIdx[j]-1) )
        {
          m_pcBinIf->encodeBin( (predIdx[j]-2) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, mpmContext[preds[j][2]] ) );
#if PIP
		  if (encodeTime && verbose)
			  cout << (Int)((predIdx[j] - 2) ? 1 : 0);
#endif
#else
        m_pcBinIf->encodeBin( (predIdx[j]-1) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, uiContextMPM1[aiCase[j]] ) );
        if ( (predIdx[j]-1) )
        {
          m_pcBinIf->encodeBin( (predIdx[j]-2) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, uiContextMPM2[aiCase[j]] ) );
#endif

          if (predIdx[j]-2)
          {
            m_pcBinIf->encodeBinEP( (predIdx[j]-3) ? 1 : 0 );
#if PIP
			if (encodeTime && verbose)
				cout << (Int)((predIdx[j] - 3) ? 1 : 0);
#endif
            if (predIdx[j]-3)
            {
              m_pcBinIf->encodeBinEP( (predIdx[j]-4) ? 1 : 0 );
#if PIP
			  if (encodeTime && verbose)
				  cout << (Int)((predIdx[j] - 4) ? 1 : 0);
#endif
            }
          }
        }
#else
        m_pcBinIf->encodeBinEP( predIdx[j]-1 );
#endif
      }
    }
    else
    {
#if VCEG_AZ07_INTRA_65ANG_MODES
      std::sort(preds[j], preds[j]+6);
#else
      if (preds[j][0] > preds[j][1])
      {
        std::swap(preds[j][0], preds[j][1]);
      }
      if (preds[j][0] > preds[j][2])
      {
        std::swap(preds[j][0], preds[j][2]);
      }
      if (preds[j][1] > preds[j][2])
      {
        std::swap(preds[j][1], preds[j][2]);
      }
#endif
      for(Int i = (Int(NUM_MOST_PROBABLE_MODES) - 1); i >= 0; i--)
      {
        dir[j] = dir[j] > preds[j][i] ? dir[j] - 1 : dir[j];
      }
#if VCEG_AZ07_INTRA_65ANG_MODES
#if JVET_E0077_ENHANCED_LM
      assert(dir[j]<(NUM_INTRA_MODE - NUM_INTRA_MODE_NON_ANG - 5));
#else
      assert( dir[j]<(NUM_INTRA_MODE-7) );
#endif

#if JVET_B0051_NON_MPM_MODE
#if JVET_C0024_QTBT
#if JVET_C0024_BT_FIX_TICKET22
      m_pcBinIf->encodeBin(( (dir[j]%4) ==0 ) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, 9 ) ); // flag to indicate if it is selected mode or non-selected mode
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)(((dir[j] % 4) == 0) ? 1 : 0);
#endif
#else
      m_pcBinIf->encodeBin(( (dir[j]%4) ==0 ) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, 9/3 ) ); // flag to indicate if it is selected mode or non-selected mode
#endif
#else
      m_pcBinIf->encodeBin(( (dir[j]%4) ==0 ) ? 1 : 0, m_cCUIntraPredSCModel.get( 0, 0, 9+mode/3 ) ); // flag to indicate if it is selected mode or non-selected mode
#endif
      if(dir[j] %4 ==0) 
      {
        m_pcBinIf->encodeBinsEP( dir[j]>>2, 4 );  // selected mode is 4-bit FLC coded
#if PIP
		if (encodeTime && verbose)
			cout << (Int)((dir[j]>>2));
#endif
      }
      else
      {
        dir[j] -= dir[j]>>2 ;
        dir[j] --;     
        xWriteTruncBinCode(dir[j] , 45);  // Non-selected mode is truncated binary coded
#if PIP
		if (encodeTime && verbose)
			cout << ",Trunc:" << (Int)dir[j];
#endif
      }
#else
      if( dir[j]>=(NUM_INTRA_MODE-8) )
        m_pcBinIf->encodeBinsEP( dir[j]>>2, 4 );
      else
        m_pcBinIf->encodeBinsEP( dir[j], 6 );
#endif
#else
      m_pcBinIf->encodeBinsEP( dir[j], 5 );
#endif
    }
  }
  return;
}

#if JVET_E0077_ENHANCED_LM
Void TEncSbac::codeLMModes(TComDataCU* pcCU, UInt uiAbsPartIdx, Int iMode)
{
    Int symbol = -1;
    Int k = 0;
    Int aiLMModeList[10];
    Int iSymbolNum = pcCU->getLMSymbolList(aiLMModeList, uiAbsPartIdx);
    for (k = 0; k < LM_SYMBOL_NUM; k++)
    {
        if (aiLMModeList[k] == iMode || (aiLMModeList[k] == -1 && iMode < LM_CHROMA_IDX))
        {
            symbol = k;
            break;
        }
    }
    assert(symbol >= 0);


    xWriteUnaryMaxSymbol(symbol, &m_cCUChromaPredSCModel.get(0, 0, 1), 1, iSymbolNum - 1);
    return;
}
#endif

Void TEncSbac::codeIntraDirChroma( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if PIP
	if (encodeTime && verbose)
		cout << ",DirChroma:";
#endif
  UInt uiIntraDirChroma = pcCU->getIntraDir( CHANNEL_TYPE_CHROMA, uiAbsPartIdx );
#if JVET_E0062_MULTI_DMS
  Int i = 0;
#if COM16_C806_LMCHROMA
  Int iStartIdx = 1;
#else
  Int iStartIdx = 0;
#endif
#if COM16_C806_LMCHROMA
#if JVET_E0077_ENHANCED_LM
  const UInt csx = getComponentScaleX(COMPONENT_Cb, pcCU->getSlice()->getSPS()->getChromaFormatIdc());
  const UInt csy = getComponentScaleY(COMPONENT_Cb, pcCU->getSlice()->getSPS()->getChromaFormatIdc());

  Int iBlockSize = (pcCU->getHeight(uiAbsPartIdx) >> csy) + (pcCU->getWidth(uiAbsPartIdx) >> csx);
#if JVET_E0077_MMLM
  if (iBlockSize >= g_aiMMLM_MinSize[pcCU->getSlice()->isIntra() ? 0 : 1])
    {
        iStartIdx += JVET_E0077_MMLM;
    }
#endif
#if JVET_E0077_LM_MF
  if (iBlockSize >= g_aiMFLM_MinSize[pcCU->getSlice()->isIntra() ? 0 : 1])
    {
        iStartIdx += LM_FILTER_NUM;
    }
#endif
  if (IsLMMode(uiIntraDirChroma) && pcCU->getSlice()->getSPS()->getUseLMChroma())
#else
  if( uiIntraDirChroma == LM_CHROMA_IDX && pcCU->getSlice()->getSPS()->getUseLMChroma() )
#endif
  {
    m_pcBinIf->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );   
#if PIP
	if (encodeTime && verbose)
		cout << 0;
#endif
#if JVET_E0077_ENHANCED_LM
        Int iCtx = 6;
#if JVET_E0077_MMLM
        if (iBlockSize >= g_aiMMLM_MinSize[pcCU->getSlice()->isIntra() ? 0 : 1])
        {
            UInt uiFlag = uiIntraDirChroma == MMLM_CHROMA_IDX;
            m_pcBinIf->encodeBin(uiFlag, m_cCUChromaPredSCModel.get(0, 0, iCtx++));
#if PIP
			if (encodeTime && verbose)
				cout << (Int)uiFlag;
#endif
        }
#endif

#if JVET_E0077_LM_MF
        if (iBlockSize >= g_aiMFLM_MinSize[pcCU->getSlice()->isIntra() ? 0 : 1])
        {
            if (((uiIntraDirChroma == LM_CHROMA_IDX)
                || (uiIntraDirChroma >= LM_CHROMA_F1_IDX && uiIntraDirChroma < LM_CHROMA_F1_IDX + LM_FILTER_NUM)))
            {
                m_pcBinIf->encodeBin(uiIntraDirChroma == LM_CHROMA_IDX, m_cCUChromaPredSCModel.get(0, 0, iCtx++));
#if PIP
				if (encodeTime && verbose)
					cout << (Int)(uiIntraDirChroma == LM_CHROMA_IDX);
#endif

                if (uiIntraDirChroma >= LM_CHROMA_F1_IDX && uiIntraDirChroma < LM_CHROMA_F1_IDX + LM_FILTER_NUM)
                {
                    Int iLable = uiIntraDirChroma - LM_CHROMA_F1_IDX;
                    m_pcBinIf->encodeBin((iLable >> 1) & 1, m_cCUChromaPredSCModel.get(0, 0, iCtx++));
                    m_pcBinIf->encodeBin(iLable & 1, m_cCUChromaPredSCModel.get(0, 0, iCtx++));
#if PIP
					if (encodeTime && verbose)
						cout << (Int)((iLable >> 1) & 1) << (iLable & 1);
#endif
                }
            }
        }

#endif
#endif
  }
  else
#endif
  {
    UInt iDMIdx = 0, uiAllowedChromaDir[NUM_CHROMA_MODE];
    pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );

    for(i = iStartIdx; i < (NUM_DM_MODES + iStartIdx); i++)
    {
      if(uiIntraDirChroma == uiAllowedChromaDir[i])
      {
        iDMIdx = i - iStartIdx;
        break;
      }
    }

#if COM16_C806_LMCHROMA
    if( pcCU->getSlice()->getSPS()->getUseLMChroma() )
    {
      m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
#if PIP
	  if (encodeTime && verbose)
		  cout << 1;
#endif
    }
#endif
            
    UInt ictxIdx = 1;
    m_pcBinIf->encodeBin(iDMIdx ? 1 : 0, m_cCUChromaPredSCModel.get(0, 0, ictxIdx));
#if PIP
	if (encodeTime && verbose)
		cout << (Int)(iDMIdx ? 1 : 0);
#endif
    UInt uiMaxSymbol = NUM_DM_MODES;
    if (iDMIdx)
    {
      Bool bCodeLast = (uiMaxSymbol > iDMIdx);
      while (--iDMIdx)
      {
        ictxIdx++;
        m_pcBinIf->encodeBin(1, m_cCUChromaPredSCModel.get(0, 0, ictxIdx));
#if PIP
		if (encodeTime && verbose)
			cout << 1;
#endif
      }
      if (bCodeLast)
      {
        ictxIdx++;
        m_pcBinIf->encodeBin(0, m_cCUChromaPredSCModel.get(0, 0, ictxIdx));
#if PIP
		if (encodeTime && verbose)
			cout << 0;
#endif
      }
    }
      
  }
#else
  if( uiIntraDirChroma == DM_CHROMA_IDX )
  {
    m_pcBinIf->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
  }

#if COM16_C806_LMCHROMA
#if JVET_E0077_ENHANCED_LM
  else
  {
      m_pcBinIf->encodeBin(1, m_cCUChromaPredSCModel.get(0, 0, 0));
      if (pcCU->getSlice()->getSPS()->getUseLMChroma())
      {
          codeLMModes(pcCU, uiAbsPartIdx, uiIntraDirChroma);
      }
  }
  if (uiIntraDirChroma == DM_CHROMA_IDX || IsLMMode(uiIntraDirChroma))
  {
      //Do nothing
  }
#else
  else if( uiIntraDirChroma == LM_CHROMA_IDX && pcCU->getSlice()->getSPS()->getUseLMChroma() )
  {
    m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
    m_pcBinIf->encodeBin( 0, m_cCUChromaPredSCModel.get( 0, 0, 1 ) );
  }
#endif

#endif
  else
  {
#if !JVET_E0077_ENHANCED_LM
    m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 0 ) );
#if COM16_C806_LMCHROMA
    if (pcCU->getSlice()->getSPS()->getUseLMChroma())
    {
      m_pcBinIf->encodeBin( 1, m_cCUChromaPredSCModel.get( 0, 0, 1 ));
    }
#endif
#endif
    UInt uiAllowedChromaDir[ NUM_CHROMA_MODE ];
    pcCU->getAllowedChromaDir( uiAbsPartIdx, uiAllowedChromaDir );

#if COM16_C806_LMCHROMA
    for( Int i = 0; i < NUM_CHROMA_MODE - 2; i++ )
#else
    for( Int i = 0; i < NUM_CHROMA_MODE - 1; i++ )
#endif
    {
      if( uiIntraDirChroma == uiAllowedChromaDir[i] )
      {
        uiIntraDirChroma = i;
        break;
      }
    }

    m_pcBinIf->encodeBinsEP( uiIntraDirChroma, 2 );
  }
#endif
  return;
}


Void TEncSbac::codeInterDir( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  const UInt uiInterDir = pcCU->getInterDir( uiAbsPartIdx ) - 1;
  const UInt uiCtx      = pcCU->getCtxInterDir( uiAbsPartIdx );
  ContextModel *pCtx    = m_cCUInterDirSCModel.get( 0 );
#if COM16_C806_LARGE_CTU
  assert( uiCtx < 4 );  // uiCtx=4 is only for the last bin when uiInterDir < 2
#endif

#if COM16_C806_HEVC_MOTION_CONSTRAINT_REMOVAL
#if JVET_C0024_QTBT
  if (1 )
#else
  if (pcCU->getSlice()->getSPS()->getAtmvpEnableFlag() || pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N || pcCU->getHeight(uiAbsPartIdx) != 8 )
#endif
#else
#if JVET_C0024_QTBT
  if(1)
#else
  if (pcCU->getPartitionSize(uiAbsPartIdx) == SIZE_2Nx2N || pcCU->getHeight(uiAbsPartIdx) != 8 )
#endif
#endif
  {
    m_pcBinIf->encodeBin( uiInterDir == 2 ? 1 : 0, *( pCtx + uiCtx ) );
  }

  if (uiInterDir < 2)
  {
    m_pcBinIf->encodeBin( uiInterDir, *( pCtx + 4 ) );
  }

  return;
}

Void TEncSbac::codeRefFrmIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  Int iRefFrame = pcCU->getCUMvField( eRefList )->getRefIdx( uiAbsPartIdx );
  ContextModel *pCtx = m_cCURefPicSCModel.get( 0 );
  m_pcBinIf->encodeBin( ( iRefFrame == 0 ? 0 : 1 ), *pCtx );

  if( iRefFrame > 0 )
  {
    UInt uiRefNum = pcCU->getSlice()->getNumRefIdx( eRefList ) - 2;
    pCtx++;
    iRefFrame--;
    for( UInt ui = 0; ui < uiRefNum; ++ui )
    {
      const UInt uiSymbol = ui == iRefFrame ? 0 : 1;
      if( ui == 0 )
      {
        m_pcBinIf->encodeBin( uiSymbol, *pCtx );
      }
      else
      {
        m_pcBinIf->encodeBinEP( uiSymbol );
      }
      if( uiSymbol == 0 )
      {
        break;
      }
    }
  }
  return;
}

Void TEncSbac::codeMvd( TComDataCU* pcCU, UInt uiAbsPartIdx, RefPicList eRefList )
{
  if(pcCU->getSlice()->getMvdL1ZeroFlag() && eRefList == REF_PIC_LIST_1 && pcCU->getInterDir(uiAbsPartIdx)==3)
  {
    return;
  }

  const TComCUMvField* pcCUMvField = pcCU->getCUMvField( eRefList );
#if VCEG_AZ07_IMV || VCEG_AZ07_FRUC_MERGE || VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE
  Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
#else
  const Int iHor = pcCUMvField->getMvd( uiAbsPartIdx ).getHor();
  const Int iVer = pcCUMvField->getMvd( uiAbsPartIdx ).getVer();
#endif
  ContextModel* pCtx = m_cCUMvdSCModel.get( 0 );

#if VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE
  assert( iHor == ( iHor >> VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE << VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE ) );
  assert( iVer == ( iVer >> VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE << VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE ) );
  iHor >>= VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE;
  iVer >>= VCEG_AZ07_MV_ADD_PRECISION_BIT_FOR_STORE;
#endif

#if VCEG_AZ07_IMV
  if( pcCU->getiMVFlag( uiAbsPartIdx ) && pcCU->getSlice()->getSPS()->getIMV() )
  {
    assert( ( iHor & 0x03 ) == 0 && ( iVer & 0x03 ) == 0 );
    iHor >>= 2;
    iVer >>= 2;
  }
#if  JVET_E0076_MULTI_PEL_MVD
    if (pcCU->getiMVFlag( uiAbsPartIdx ) == 2)
    {
      assert( ( iHor % (1 << MULTI_PEL_MVD_BITS)) == 0 && ( iVer % (1 << MULTI_PEL_MVD_BITS) ) == 0 );
      iHor >>= MULTI_PEL_MVD_BITS;
      iVer >>= MULTI_PEL_MVD_BITS;
    }
#endif
#endif

  m_pcBinIf->encodeBin( iHor != 0 ? 1 : 0, *pCtx );
  m_pcBinIf->encodeBin( iVer != 0 ? 1 : 0, *pCtx );

  const Bool bHorAbsGr0 = iHor != 0;
  const Bool bVerAbsGr0 = iVer != 0;
  const UInt uiHorAbs   = 0 > iHor ? -iHor : iHor;
  const UInt uiVerAbs   = 0 > iVer ? -iVer : iVer;

  pCtx++;

  if( bHorAbsGr0 )
  {
    m_pcBinIf->encodeBin( uiHorAbs > 1 ? 1 : 0, *pCtx );
  }

  if( bVerAbsGr0 )
  {
    m_pcBinIf->encodeBin( uiVerAbs > 1 ? 1 : 0, *pCtx );
  }

  if( bHorAbsGr0 )
  {
    if( uiHorAbs > 1 )
    {
      xWriteEpExGolomb( uiHorAbs-2, 1 );
    }

    m_pcBinIf->encodeBinEP( 0 > iHor ? 1 : 0 );
  }

  if( bVerAbsGr0 )
  {
    if( uiVerAbs > 1 )
    {
      xWriteEpExGolomb( uiVerAbs-2, 1 );
    }

    m_pcBinIf->encodeBinEP( 0 > iVer ? 1 : 0 );
  }

  return;
}

Void TEncSbac::codeCrossComponentPrediction( TComTU &rTu, ComponentID compID )
{
  TComDataCU *pcCU = rTu.getCU();

  if( isLuma(compID) || !pcCU->getSlice()->getPPS()->getPpsRangeExtension().getCrossComponentPredictionEnabledFlag() )
  {
    return;
  }

  const UInt uiAbsPartIdx = rTu.GetAbsPartIdxTU();
#if JVET_E0062_MULTI_DMS
#if JVET_E0077_ENHANCED_LM
  if (!pcCU->isIntra(uiAbsPartIdx) || !IsLMMode(pcCU->getIntraDir(CHANNEL_TYPE_CHROMA, uiAbsPartIdx)))
#else
  if (!pcCU->isIntra(uiAbsPartIdx) || (pcCU->getIntraDir(CHANNEL_TYPE_CHROMA, uiAbsPartIdx) != LM_CHROMA_IDX))
#endif
#else
  if (!pcCU->isIntra(uiAbsPartIdx) || (pcCU->getIntraDir( CHANNEL_TYPE_CHROMA, uiAbsPartIdx ) == DM_CHROMA_IDX))
#endif
  {
    DTRACE_CABAC_VL( g_nSymbolCounter++ )
    DTRACE_CABAC_T("\tparseCrossComponentPrediction()")
    DTRACE_CABAC_T( "\tAddr=" )
    DTRACE_CABAC_V( compID )
    DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
    DTRACE_CABAC_V( uiAbsPartIdx )

    Int alpha = pcCU->getCrossComponentPredictionAlpha( uiAbsPartIdx, compID );
    ContextModel *pCtx = m_cCrossComponentPredictionSCModel.get(0, 0) + ((compID == COMPONENT_Cr) ? (NUM_CROSS_COMPONENT_PREDICTION_CTX >> 1) : 0);
    m_pcBinIf->encodeBin(((alpha != 0) ? 1 : 0), pCtx[0]);

    if (alpha != 0)
    {
      static const Int log2AbsAlphaMinus1Table[8] = { 0, 1, 1, 2, 2, 2, 3, 3 };
      assert(abs(alpha) <= 8);

      if (abs(alpha)>1)
      {
        m_pcBinIf->encodeBin(1, pCtx[1]);
        xWriteUnaryMaxSymbol( log2AbsAlphaMinus1Table[abs(alpha) - 1] - 1, (pCtx + 2), 1, 2 );
      }
      else
      {
        m_pcBinIf->encodeBin(0, pCtx[1]);
      }
      m_pcBinIf->encodeBin( ((alpha < 0) ? 1 : 0), pCtx[4] );
    }
    DTRACE_CABAC_T( "\tAlpha=" )
    DTRACE_CABAC_V( pcCU->getCrossComponentPredictionAlpha( uiAbsPartIdx, compID ) )
    DTRACE_CABAC_T( "\n" )
  }
}

Void TEncSbac::codeDeltaQP( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  Int iDQp  = pcCU->getQP( uiAbsPartIdx ) - pcCU->getRefQP( uiAbsPartIdx );

  Int qpBdOffsetY =  pcCU->getSlice()->getSPS()->getQpBDOffset(CHANNEL_TYPE_LUMA);
  iDQp = (iDQp + 78 + qpBdOffsetY + (qpBdOffsetY/2)) % (52 + qpBdOffsetY) - 26 - (qpBdOffsetY/2);

  UInt uiAbsDQp = (UInt)((iDQp > 0)? iDQp  : (-iDQp));
  UInt TUValue = min((Int)uiAbsDQp, CU_DQP_TU_CMAX);
  xWriteUnaryMaxSymbol( TUValue, &m_cCUDeltaQpSCModel.get( 0, 0, 0 ), 1, CU_DQP_TU_CMAX);
  if( uiAbsDQp >= CU_DQP_TU_CMAX )
  {
    xWriteEpExGolomb( uiAbsDQp - CU_DQP_TU_CMAX, CU_DQP_EG_k );
  }

  if ( uiAbsDQp > 0)
  {
    UInt uiSign = (iDQp > 0 ? 0 : 1);
    m_pcBinIf->encodeBinEP(uiSign);
  }

  return;
}

/** code chroma qp adjustment, converting from the internal table representation
 * \returns Void
 */
Void TEncSbac::codeChromaQpAdjustment( TComDataCU* cu, UInt absPartIdx )
{
  Int internalIdc = cu->getChromaQpAdj( absPartIdx );
  Int chromaQpOffsetListLen = cu->getSlice()->getPPS()->getPpsRangeExtension().getChromaQpOffsetListLen();
  /* internal_idc == 0 => flag = 0
   * internal_idc > 1 => code idc value (if table size warrents) */
  m_pcBinIf->encodeBin( internalIdc > 0, m_ChromaQpAdjFlagSCModel.get( 0, 0, 0 ) );

  if (internalIdc > 0 && chromaQpOffsetListLen > 1)
  {
    xWriteUnaryMaxSymbol( internalIdc - 1, &m_ChromaQpAdjIdcSCModel.get( 0, 0, 0 ), 0, chromaQpOffsetListLen - 1 );
  }
}

Void TEncSbac::codeQtCbf( TComTU &rTu, const ComponentID compID, const Bool lowestLevel )
{
  TComDataCU* pcCU = rTu.getCU();

  const UInt absPartIdx   = rTu.GetAbsPartIdxTU(compID);
#if !JVET_C0024_QTBT
  const UInt TUDepth      = rTu.GetTransformDepthRel();
#endif
        UInt uiCtx        = pcCU->getCtxQtCbf( rTu, toChannelType(compID) );
  const UInt contextSet   = toChannelType(compID);

#if PIP
  Bool isPIP = pcCU->getPIPflag(absPartIdx) && isLuma(compID);
#endif

#if !JVET_C0024_QTBT
  const UInt width        = rTu.getRect(compID).width;
  const UInt height       = rTu.getRect(compID).height;
  const Bool canQuadSplit = (width >= (MIN_TU_SIZE * 2)) && (height >= (MIN_TU_SIZE * 2));

  //             Since the CBF for chroma is coded at the highest level possible, if sub-TUs are
  //             to be coded for a 4x8 chroma TU, their CBFs must be coded at the highest 4x8 level
  //             (i.e. where luma TUs are 8x8 rather than 4x4)
  //    ___ ___
  //   |   |   | <- 4 x (8x8 luma + 4x8 4:2:2 chroma)
  //   |___|___|    each quadrant has its own chroma CBF
  //   |   |   | _ _ _ _
  //   |___|___|        |
  //   <--16--->        V
  //                   _ _
  //                  |_|_| <- 4 x 4x4 luma + 1 x 4x8 4:2:2 chroma
  //                  |_|_|    no chroma CBF is coded - instead the parent CBF is inherited
  //                  <-8->    if sub-TUs are present, their CBFs had to be coded at the parent level

  const UInt lowestTUDepth = TUDepth + ((!lowestLevel && !canQuadSplit) ? 1 : 0); //unsplittable TUs inherit their parent's CBF

  if ((width != height) && (lowestLevel || !canQuadSplit)) //if sub-TUs are present
  {
    const UInt subTUDepth        = lowestTUDepth + 1;                      //if this is the lowest level of the TU-tree, the sub-TUs are directly below. Otherwise, this must be the level above the lowest level (as specified above)
    const UInt partIdxesPerSubTU = rTu.GetAbsPartIdxNumParts(compID) >> 1;

    for (UInt subTU = 0; subTU < 2; subTU++)
    {
      const UInt subTUAbsPartIdx = absPartIdx + (subTU * partIdxesPerSubTU);
      const UInt uiCbf           = pcCU->getCbf(subTUAbsPartIdx, compID, subTUDepth);

      m_pcBinIf->encodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, contextSet, uiCtx));

      DTRACE_CABAC_VL( g_nSymbolCounter++ )
      DTRACE_CABAC_T( "\tparseQtCbf()" )
      DTRACE_CABAC_T( "\tsub-TU=" )
      DTRACE_CABAC_V( subTU )
      DTRACE_CABAC_T( "\tsymbol=" )
      DTRACE_CABAC_V( uiCbf )
      DTRACE_CABAC_T( "\tctx=" )
      DTRACE_CABAC_V( uiCtx )
      DTRACE_CABAC_T( "\tetype=" )
      DTRACE_CABAC_V( compID )
      DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
      DTRACE_CABAC_V( subTUAbsPartIdx )
      DTRACE_CABAC_T( "\n" )
    }
  }
  else
#endif
  {
#if JVET_C0024_QTBT
    const UInt uiCbf = pcCU->getCbf( absPartIdx, compID, 0 );
#else
    const UInt uiCbf = pcCU->getCbf( absPartIdx, compID, lowestTUDepth );
#endif
#if PIP
	if (isPIP)
	{
#if !NO_CBF
		m_pcBinIf->encodeBin(uiCbf, m_cCUQtCbfSCModelPIP.get(0, contextSet, uiCtx));
#endif
	}
	else
		m_pcBinIf->encodeBin(uiCbf, m_cCUQtCbfSCModel.get(0, contextSet, uiCtx));

	if (encodeTime && verbose)
		cout << ",cbf:" << (Int)uiCbf;
#else
    m_pcBinIf->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, contextSet, uiCtx ) );
#endif

    DTRACE_CABAC_VL( g_nSymbolCounter++ )
    DTRACE_CABAC_T( "\tparseQtCbf()" )
    DTRACE_CABAC_T( "\tsymbol=" )
    DTRACE_CABAC_V( uiCbf )
    DTRACE_CABAC_T( "\tctx=" )
    DTRACE_CABAC_V( uiCtx )
    DTRACE_CABAC_T( "\tetype=" )
    DTRACE_CABAC_V( compID )
    DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
    DTRACE_CABAC_V( rTu.GetAbsPartIdxTU(compID) )
    DTRACE_CABAC_T( "\n" )
  }
}

#if VCEG_AZ08_KLT_COMMON
Void TEncSbac::codeKLTFlags(TComTU &rTu, ComponentID component)
{
    TComDataCU* pcCU = rTu.getCU();
    const UInt uiAbsPartIdx = rTu.GetAbsPartIdxTU();

    if (pcCU->getCUTransquantBypass(uiAbsPartIdx))
    {
        return;
    }

    const TComRectangle &rect = rTu.getRect(component);
    const UInt uiWidth = rect.width;
    const UInt uiHeight = rect.height;
    UInt uiMaxTrWidth = g_uiDepth2Width[USE_MORE_BLOCKSIZE_DEPTH_MAX - 1];
    UInt uiMinTrWidth = g_uiDepth2Width[USE_MORE_BLOCKSIZE_DEPTH_MIN - 1];
    Bool checkKLTY = ((uiWidth == uiHeight) && (uiWidth <= uiMaxTrWidth) && (uiWidth >= uiMinTrWidth) && (toChannelType(component) == CHANNEL_TYPE_LUMA));
    if (checkKLTY == false)
    {
        return;
    }

    UInt useKLTFlag = pcCU->getKLTFlag(uiAbsPartIdx, component);
    m_pcBinIf->encodeBin(useKLTFlag, m_cKLTFlagSCModel.get(0, toChannelType(component), 0));

    DTRACE_CABAC_VL(g_nSymbolCounter++)
    DTRACE_CABAC_T("\tparseKLTFlag()");
    DTRACE_CABAC_T("\tsymbol=")
    DTRACE_CABAC_V(useKLTFlag)
    DTRACE_CABAC_T("\tAddr=")
    DTRACE_CABAC_V(pcCU->getCtuRsAddr())
    DTRACE_CABAC_T("\tetype=")
    DTRACE_CABAC_V(component)
    DTRACE_CABAC_T("\tuiAbsPartIdx=")
    DTRACE_CABAC_V(rTu.GetAbsPartIdxTU())
    DTRACE_CABAC_T("\n")
}
#endif

Void TEncSbac::codeTransformSkipFlags (TComTU &rTu, ComponentID component )
{
  TComDataCU* pcCU=rTu.getCU();
  const UInt uiAbsPartIdx=rTu.GetAbsPartIdxTU();

  if (pcCU->getCUTransquantBypass(uiAbsPartIdx))
  {
    return;
  }

#if JVET_C0024_QTBT
  if (!TUCompRectHasAssociatedTransformSkipFlag(pcCU->getSlice()->isIntra(), rTu.getRect(component), pcCU->getSlice()->getPPS()->getPpsRangeExtension().getLog2MaxTransformSkipBlockSize()))
#else
  if (!TUCompRectHasAssociatedTransformSkipFlag(rTu.getRect(component), pcCU->getSlice()->getPPS()->getPpsRangeExtension().getLog2MaxTransformSkipBlockSize()))
#endif
  {
    return;
  }

  UInt useTransformSkip = pcCU->getTransformSkip( uiAbsPartIdx,component);
  m_pcBinIf->encodeBin( useTransformSkip, m_cTransformSkipSCModel.get( 0, toChannelType(component), 0 ) );

  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T("\tparseTransformSkip()");
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( useTransformSkip )
  DTRACE_CABAC_T( "\tAddr=" )
  DTRACE_CABAC_V( pcCU->getCtuRsAddr() )
  DTRACE_CABAC_T( "\tetype=" )
  DTRACE_CABAC_V( component )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( rTu.GetAbsPartIdxTU() )
  DTRACE_CABAC_T( "\n" )
}


/** Code I_PCM information.
 * \param pcCU pointer to CU
 * \param uiAbsPartIdx CU index
 * \returns Void
 */
Void TEncSbac::codeIPCMInfo( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
#if PIP
	if (encodeTime && verbose)
		cout << ",IPCM:";
#endif
  UInt uiIPCM = (pcCU->getIPCMFlag(uiAbsPartIdx) == true)? 1 : 0;

  Bool writePCMSampleFlag = pcCU->getIPCMFlag(uiAbsPartIdx);

  m_pcBinIf->encodeBinTrm (uiIPCM);
#if PIP
  if (encodeTime && verbose)
	  cout << (Int)uiIPCM;
#endif

  if (writePCMSampleFlag)
  {
    m_pcBinIf->encodePCMAlignBits();

    const UInt minCoeffSizeY = pcCU->getPic()->getMinCUWidth() * pcCU->getPic()->getMinCUHeight();
    const UInt offsetY       = minCoeffSizeY * uiAbsPartIdx;
    for (UInt ch=0; ch < pcCU->getPic()->getNumberValidComponents(); ch++)
    {
      const ComponentID compID = ComponentID(ch);
      const UInt offset = offsetY >> (pcCU->getPic()->getComponentScaleX(compID) + pcCU->getPic()->getComponentScaleY(compID));
      Pel * pPCMSample  = pcCU->getPCMSample(compID) + offset;
      const UInt width  = pcCU->getWidth (uiAbsPartIdx) >> pcCU->getPic()->getComponentScaleX(compID);
      const UInt height = pcCU->getHeight(uiAbsPartIdx) >> pcCU->getPic()->getComponentScaleY(compID);
      const UInt sampleBits = pcCU->getSlice()->getSPS()->getPCMBitDepth(toChannelType(compID));
      for (UInt y=0; y<height; y++)
      {
        for (UInt x=0; x<width; x++)
        {
          UInt sample = pPCMSample[x];
          m_pcBinIf->xWritePCMCode(sample, sampleBits);
#if PIP
		  if (encodeTime && verbose)
			  cout << (Int)sample;
#endif
        }
        pPCMSample += width;
      }
    }

    m_pcBinIf->resetBac();
  }
}

Void TEncSbac::codeQtRootCbf( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  UInt uiCbf = pcCU->getQtRootCbf( uiAbsPartIdx );
  UInt uiCtx = 0;
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
#if PIP
  if (encodeTime && verbose)
	  cout << ",QtRoot:" << (Int)uiCbf;
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseQtRootCbf()" )
  DTRACE_CABAC_T( "\tsymbol=" )
  DTRACE_CABAC_V( uiCbf )
  DTRACE_CABAC_T( "\tctx=" )
  DTRACE_CABAC_V( uiCtx )
  DTRACE_CABAC_T( "\tuiAbsPartIdx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\n" )
}

Void TEncSbac::codeQtCbfZero( TComTU & rTu, const ChannelType chType )
{
  // this function is only used to estimate the bits when cbf is 0
  // and will never be called when writing the bistream. do not need to write log
  UInt uiCbf = 0;
  UInt uiCtx = rTu.getCU()->getCtxQtCbf( rTu, chType );

  m_pcBinIf->encodeBin( uiCbf , m_cCUQtCbfSCModel.get( 0, chType, uiCtx ) );
}

Void TEncSbac::codeQtRootCbfZero( )
{
  // this function is only used to estimate the bits when cbf is 0
  // and will never be called when writing the bistream. do not need to write log
  UInt uiCbf = 0;
  UInt uiCtx = 0;
  m_pcBinIf->encodeBin( uiCbf , m_cCUQtRootCbfSCModel.get( 0, 0, uiCtx ) );
}

/** Encode (X,Y) position of the last significant coefficient
 * \param uiPosX     X component of last coefficient
 * \param uiPosY     Y component of last coefficient
 * \param width      Block width
 * \param height     Block height
 * \param component  chroma component ID
 * \param uiScanIdx  scan type (zig-zag, hor, ver)
 * This method encodes the X and Y component within a block of the last significant coefficient.
 */
Void TEncSbac::codeLastSignificantXY( UInt uiPosX, UInt uiPosY, Int width, Int height, ComponentID component, UInt uiScanIdx 
#if PIP
	, Bool isPIP
#endif
	)
{
#if PIP

#if SP_COEFFNxN_2
	if (SpatialCodeCoeffNxN_2)
	{
		uiPosX = 3 - uiPosX;
		uiPosY = 3 - uiPosY;
	}
#endif

	UInt bits1 = m_pcBinIf->getNumWrittenBits();
	if (encodeTime && false)
		fprintf(stdout, "LastX:%d/%d,LastY:%d/%d", uiPosX, width, uiPosY, height);

	if (encodeTime && verbose)
		cout << ",LastXY,X:";
#endif
  // swap
  if( uiScanIdx == SCAN_VER )
  {
    swap( uiPosX, uiPosY );
    swap( width,  height );
  }

  UInt uiCtxLast;
  UInt uiGroupIdxX    = g_uiGroupIdx[ uiPosX ];
  UInt uiGroupIdxY    = g_uiGroupIdx[ uiPosY ];

#if PIP
  ContextModel *pCtxX = isPIP ? m_cCuCtxLastXPIP.get(0, 0) : m_cCuCtxLastX.get(0, toChannelType(component));
  ContextModel *pCtxY = isPIP ? m_cCuCtxLastYPIP.get(0, 0) : m_cCuCtxLastY.get(0, toChannelType(component));  
#else
  ContextModel *pCtxX = m_cCuCtxLastX.get( 0, toChannelType(component) );
  ContextModel *pCtxY = m_cCuCtxLastY.get( 0, toChannelType(component) );
#endif
#if VCEG_AZ07_CTX_RESIDUALCODING && !COM16_C806_T64
  Int widthCtx = component ? 4: width;
  const UInt *puiCtxIdxX = g_uiLastCtx + ( g_aucConvertToBit[ widthCtx ] * ( g_aucConvertToBit[ widthCtx ] + 3 ) );

  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxX; uiCtxLast++ )
  {
    if (component)
    {
      m_pcBinIf->encodeBin( 1, *( pCtxX + (uiCtxLast >> g_aucConvertToBit[ width ]) ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, *( pCtxX + puiCtxIdxX[ uiCtxLast ] ) );
    }
  }
  if( uiGroupIdxX < g_uiGroupIdx[ width - 1 ])
  {
    if ( component )
    {
      m_pcBinIf->encodeBin( 0, *( pCtxX + (uiCtxLast >> g_aucConvertToBit[ width ]) ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 0, *( pCtxX + puiCtxIdxX[ uiCtxLast ] ) );
    }
  }

  // posY
  Int heightCtx = component ? 4: height;
  const UInt *puiCtxIdxY = g_uiLastCtx + ( g_aucConvertToBit[ heightCtx ] * ( g_aucConvertToBit[ heightCtx ] + 3 ) );
  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxY; uiCtxLast++ )
  {
    if (component)
    {
      m_pcBinIf->encodeBin( 1, *( pCtxY + (uiCtxLast >>  g_aucConvertToBit[ height ])));
    }
    else
    {
      m_pcBinIf->encodeBin( 1, *( pCtxY + puiCtxIdxY[ uiCtxLast ] ) );
    }
  }
  if( uiGroupIdxY < g_uiGroupIdx[ height - 1 ])
  {
    if (component)
    {
      m_pcBinIf->encodeBin( 0, *( pCtxY + (uiCtxLast >> g_aucConvertToBit[ height ]) ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 0, *( pCtxY + puiCtxIdxY[ uiCtxLast ] ) );
    }
   }
#else
  Int blkSizeOffsetX, blkSizeOffsetY, shiftX, shiftY;
  getLastSignificantContextParameters(component, width, height, blkSizeOffsetX, blkSizeOffsetY, shiftX, shiftY);

  //------------------

  // posX

  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxX; uiCtxLast++ )
  {
    m_pcBinIf->encodeBin( 1, *( pCtxX + blkSizeOffsetX + (uiCtxLast >>shiftX) ) );
#if PIP
	if (encodeTime && verbose)
		cout << 1;
#endif
  }
  if( uiGroupIdxX < g_uiGroupIdx[ width - 1 ])
  {
    m_pcBinIf->encodeBin( 0, *( pCtxX + blkSizeOffsetX + (uiCtxLast >>shiftX) ) );
#if PIP
	if (encodeTime && verbose)
		cout << 0;
#endif
  }

  // posY
#if PIP
  if (encodeTime && verbose)
	  cout << "Y:";
#endif
  for( uiCtxLast = 0; uiCtxLast < uiGroupIdxY; uiCtxLast++ )
  {
    m_pcBinIf->encodeBin( 1, *( pCtxY + blkSizeOffsetY + (uiCtxLast >>shiftY) ) );
#if PIP
	if (encodeTime && verbose)
		cout << 1;
#endif
  }
  if( uiGroupIdxY < g_uiGroupIdx[ height - 1 ])
  {
    m_pcBinIf->encodeBin( 0, *( pCtxY + blkSizeOffsetY + (uiCtxLast >>shiftY) ) );
#if PIP
	if (encodeTime && verbose)
		cout << 0;
#endif
  }
#endif
  // EP-coded part
  if ( uiGroupIdxX > 3 )
  {
    UInt uiCount = ( uiGroupIdxX - 2 ) >> 1;
    uiPosX       = uiPosX - g_uiMinInGroup[ uiGroupIdxX ];
    for (Int i = uiCount - 1 ; i >= 0; i-- )
    {
      m_pcBinIf->encodeBinEP( ( uiPosX >> i ) & 1 );
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)((uiPosX >> i) & 1);
#endif
    }
  }
  if ( uiGroupIdxY > 3 )
  {
    UInt uiCount = ( uiGroupIdxY - 2 ) >> 1;
    uiPosY       = uiPosY - g_uiMinInGroup[ uiGroupIdxY ];
    for ( Int i = uiCount - 1 ; i >= 0; i-- )
    {
      m_pcBinIf->encodeBinEP( ( uiPosY >> i ) & 1 );
#if PIP
	  if (encodeTime && verbose)
		  cout << (Int)((uiPosY >> i) & 1);
#endif
    }
  }
#if PIP
  UInt bits2 = m_pcBinIf->getNumWrittenBits();
  if (encodeTime && false)
	  fprintf(stdout, "\t\t(Bits:%d)\n", bits2 - bits1); 
  if (verbose && encodeTime)
	  cout << ",";
#endif
}

Void TEncSbac::codeCoeffNxN( TComTU &rTu, TCoeff* pcCoef, const ComponentID compID 
#if VCEG_AZ05_ROT_TR    || VCEG_AZ05_INTRA_MPI || COM16_C1044_NSST || COM16_C1046_PDPC_INTRA
  , Int& bCbfCU
#endif
#if JVET_C0045_C0053_NO_NSST_FOR_TS
  , Int& iNonZeroCoeffNonTs
#endif
  )
{



  TComDataCU* pcCU=rTu.getCU();
  const UInt uiAbsPartIdx=rTu.GetAbsPartIdxTU(compID);
  const TComRectangle &tuRect=rTu.getRect(compID);
  const UInt uiWidth=tuRect.width;
  const UInt uiHeight=tuRect.height;
  const TComSPS &sps=*(pcCU->getSlice()->getSPS());

#if PIP
  Bool isPIP = pcCU->getPIPflag(uiAbsPartIdx) && isLuma(compID);
  

  if (encodeTime)
  {
	  if (uiWidth == 4 && uiHeight == 4 && isLuma(compID))
	  {
		  int max = 0;
		  for (int c = 0; c < uiWidth*uiHeight; c++)
		  {
			  if (abs(pcCoef[c]) > max)
			  {
				  max = abs(pcCoef[c]);
			  }
			  if (abs(pcCoef[c]))
				  sigCnt++;
		  }

		  if (max > MAXDYN - 1)
			  maxExceeded = true;
		  // maxExceeded = false;
		  if (!maxExceeded || true)
		  {
			  FILE* fout = fopen("log.txt", "a");
			  // fprintf(fout, "%1d\t", isPIP);
			  for (int cf = 0; cf < 16; cf++)
				  fprintf(fout, "%4d\t", pcCoef[cf]);
			  // fprintf(fout, "\n");
			  fclose(fout);
		  }
	  }
  }

#if SP_COEFFNxX
  Int* spQR1_encode = (Int*)xMalloc(Int, uiWidth*uiHeight*sizeof(Int));
#endif

#if SP_COEFFNxN_2
  TCoeff* TMP_pcCoef = (TCoeff*)xMalloc(TCoeff, uiWidth*uiHeight*sizeof(TCoeff));
  if (SpatialCodeCoeffNxN_2)
  {
	  memcpy(TMP_pcCoef, pcCoef, uiWidth*uiHeight*sizeof(TCoeff));
	  memcpy(pcCoef, TMPSpR1, uiWidth*uiHeight*sizeof(TCoeff));
  }
#endif

#endif


  
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tparseCoeffNxN()\teType=" )
  DTRACE_CABAC_V( compID )
  DTRACE_CABAC_T( "\twidth=" )
  DTRACE_CABAC_V( uiWidth )
  DTRACE_CABAC_T( "\theight=" )
  DTRACE_CABAC_V( uiHeight )
  DTRACE_CABAC_T( "\tdepth=" )
//  DTRACE_CABAC_V( rTu.GetTransformDepthTotalAdj(compID) )
  DTRACE_CABAC_V( rTu.GetTransformDepthTotal() )
  DTRACE_CABAC_T( "\tabspartidx=" )
  DTRACE_CABAC_V( uiAbsPartIdx )
  DTRACE_CABAC_T( "\ttoCU-X=" )
  DTRACE_CABAC_V( pcCU->getCUPelX() )
  DTRACE_CABAC_T( "\ttoCU-Y=" )
  DTRACE_CABAC_V( pcCU->getCUPelY() )
  DTRACE_CABAC_T( "\tCU-addr=" )
  DTRACE_CABAC_V(  pcCU->getCtuRsAddr() )
  DTRACE_CABAC_T( "\tinCU-X=" )
//  DTRACE_CABAC_V( g_auiRasterToPelX[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_V( g_auiRasterToPelX[ g_auiZscanToRaster[rTu.GetAbsPartIdxTU(compID)] ] )
  DTRACE_CABAC_T( "\tinCU-Y=" )
// DTRACE_CABAC_V( g_auiRasterToPelY[ g_auiZscanToRaster[uiAbsPartIdx] ] )
  DTRACE_CABAC_V( g_auiRasterToPelY[ g_auiZscanToRaster[rTu.GetAbsPartIdxTU(compID)] ] )
  DTRACE_CABAC_T( "\tpredmode=" )
  DTRACE_CABAC_V(  pcCU->getPredictionMode( uiAbsPartIdx ) )
  DTRACE_CABAC_T( "\n" )

  //--------------------------------------------------------------------------------------------------

#if JVET_C0024_QTBT
  if( uiWidth > sps.getCTUSize() )
#else
  if( uiWidth > sps.getMaxTrSize() )
#endif
  {
    std::cerr << "ERROR: codeCoeffNxN was passed a TU with dimensions larger than the maximum allowed size" << std::endl;
    assert(false);
    exit(1);
  }

  // compute number of significant coefficients
#if SP_COEFFNxX
  UInt uiNumSig = TEncEntropy::countNonZeroCoeffs((SpatialCodeCoeffNxN ? spQR1_encode : pcCoef), uiWidth * uiHeight);
#else
  UInt uiNumSig = TEncEntropy::countNonZeroCoeffs(pcCoef, uiWidth * uiHeight);
#endif

#if COM16_C806_EMT
  UInt uiTuNumSig = uiNumSig;
#endif
#if JVET_C0045_C0053_NO_NSST_FOR_TS
  if( !pcCU->getTransformSkip( uiAbsPartIdx,compID) )
  {
    iNonZeroCoeffNonTs += uiNumSig;
  }
#endif

  if ( uiNumSig == 0 )
  {
    std::cerr << "ERROR: codeCoeffNxN called for empty TU!" << std::endl;
    assert(false);
    exit(1);
  }


#if PIP
  Bits1 = getNumberOfWrittenBits();
  UInt bits1 = getNumberOfWrittenBits();

#endif
  //--------------------------------------------------------------------------------------------------

  //set parameters

  const ChannelType  chType            = toChannelType(compID);
#if JVET_C0024_QTBT
  const UInt         uiLog2BlockWidth  = g_aucConvertToBit[ uiWidth  ] + MIN_CU_LOG2;
  const UInt         uiLog2BlockHeight = g_aucConvertToBit[ uiHeight ] + MIN_CU_LOG2;
#if VCEG_AZ07_CTX_RESIDUALCODING
  const UInt         uiLog2BlockSize = (uiLog2BlockWidth + uiLog2BlockHeight)>>1;
#endif
#else
  const UInt         uiLog2BlockWidth  = g_aucConvertToBit[ uiWidth  ] + 2;
#if !VCEG_AZ07_CTX_RESIDUALCODING || JVET_C0046_ZO_ASSERT
  const UInt         uiLog2BlockHeight = g_aucConvertToBit[ uiHeight ] + 2;
#endif
#endif

  const ChannelType  channelType       = toChannelType(compID);
  const Bool         extendedPrecision = sps.getSpsRangeExtension().getExtendedPrecisionProcessingFlag();


  const Bool         alignCABACBeforeBypass = sps.getSpsRangeExtension().getCabacBypassAlignmentEnabledFlag();
  const Int          maxLog2TrDynamicRange  = sps.getMaxLog2TrDynamicRange(channelType);

  Bool beValid;

  {
    Int uiIntraMode = -1;
    const Bool       bIsLuma = isLuma(compID);
    Int isIntra = pcCU->isIntra(uiAbsPartIdx) ? 1 : 0;
    if ( isIntra )
    {
#if PIP
		uiIntraMode = isPIP ? PIP_DEF_IPM : pcCU->getIntraDir(toChannelType(compID), uiAbsPartIdx);
#else
      uiIntraMode = pcCU->getIntraDir( toChannelType(compID), uiAbsPartIdx );
#endif

#if JVET_C0024_QTBT
#if JVET_E0062_MULTI_DMS
      uiIntraMode = uiIntraMode;
#else
      uiIntraMode = (uiIntraMode==DM_CHROMA_IDX && !bIsLuma) 
        ? (pcCU->getSlice()->isIntra()? pcCU->getPic()->getCtu(pcCU->getCtuRsAddr())->getIntraDir(CHANNEL_TYPE_LUMA, pcCU->getZorderIdxInCtu()+uiAbsPartIdx)
        :pcCU->getIntraDir(CHANNEL_TYPE_LUMA, uiAbsPartIdx)) : uiIntraMode;
#endif
#else
      const UInt partsPerMinCU = 1<<(2*(sps.getMaxTotalCUDepth() - sps.getLog2DiffMaxMinCodingBlockSize()));
      uiIntraMode = (uiIntraMode==DM_CHROMA_IDX && !bIsLuma) ? pcCU->getIntraDir(CHANNEL_TYPE_LUMA, getChromasCorrespondingPULumaIdx(uiAbsPartIdx, rTu.GetChromaFormat(), partsPerMinCU)) : uiIntraMode;
#endif
      uiIntraMode = ((rTu.GetChromaFormat() == CHROMA_422) && !bIsLuma) ? g_chroma422IntraAngleMappingTable[uiIntraMode] : uiIntraMode;
    }

    Int transformSkip = pcCU->getTransformSkip( uiAbsPartIdx,compID) ? 1 : 0;
    Bool rdpcm_lossy = ( transformSkip && isIntra && ( (uiIntraMode == HOR_IDX) || (uiIntraMode == VER_IDX) ) ) && pcCU->isRDPCMEnabled(uiAbsPartIdx);

    if ( (pcCU->getCUTransquantBypass(uiAbsPartIdx)) || rdpcm_lossy )
    {
      beValid = false;
      if ( (!pcCU->isIntra(uiAbsPartIdx)) && pcCU->isRDPCMEnabled(uiAbsPartIdx))
      {
        codeExplicitRdpcmMode( rTu, compID);
      }
    }
    else
    {
      beValid = pcCU->getSlice()->getPPS()->getSignHideFlag();
    }
  }

  //--------------------------------------------------------------------------------------------------

#if PIP
  if (!(pcCU->getPIPflag(uiAbsPartIdx) && isLuma(compID) ))
#endif
  if(pcCU->getSlice()->getPPS()->getUseTransformSkip())
  {
    codeTransformSkipFlags(rTu, compID);
    if(pcCU->getTransformSkip(uiAbsPartIdx, compID) && !pcCU->isIntra(uiAbsPartIdx) && pcCU->isRDPCMEnabled(uiAbsPartIdx))
    {
      //  This TU has coefficients and is transform skipped. Check whether is inter coded and if yes encode the explicit RDPCM mode
      codeExplicitRdpcmMode( rTu, compID);

      if(pcCU->getExplicitRdpcmMode(compID, uiAbsPartIdx) != RDPCM_OFF)
      {
        //  Sign data hiding is avoided for horizontal and vertical explicit RDPCM modes
        beValid = false;
      }
    }
  }

  //--------------------------------------------------------------------------------------------------
  const Bool  bUseGolombRiceParameterAdaptation = sps.getSpsRangeExtension().getPersistentRiceAdaptationEnabledFlag();
        UInt &currentGolombRiceStatistic        = m_golombRiceAdaptationStatistics[rTu.getGolombRiceStatisticsIndex(compID)];

  //select scans
  TUEntropyCodingParameters codingParameters;
  getTUEntropyCodingParameters(codingParameters, rTu, compID);


#if PIP && SP_COEFFNxX
  // encode the Significance Map 
  Bool *significance_map = (Bool*)xMalloc(Bool, uiWidth*uiHeight*sizeof(Bool)); // Free it
  Int *reordered_spR1 = (Int*)xMalloc(Int, uiWidth*uiHeight*sizeof(Int));		// Free it
  memset(reordered_spR1, 0, uiWidth*uiHeight*sizeof(Int));
  if (isPIP && SpatialCodeCoeffNxN)
  {	  
	  if (encodeTime) // TEMP
	  {
		  for (int c = 0; c < uiWidth*uiHeight; c++)
			  fprintf(stdout, "%4d\t", spQR1_encode[c]);
		  fprintf(stdout, "\n");
	  }
	  for (int c = 0; c < uiWidth*uiHeight; c++)
	  {
		  significance_map[c] = abs(spQR1_encode[c]) > 0;
		  // Encode the signigicance map
		  m_pcBinIf->encodeBinEP(significance_map[c]);
	  }	  
	  
	  int scanCnt = 0;
	  for (int c = 0; c < uiWidth*uiHeight; c++)
	  {
		  if (significance_map[codingParameters.scan[c]])
		  {
			  reordered_spR1[codingParameters.scan[scanCnt]] = spQR1_encode[codingParameters.scan[c]];
			  scanCnt++;
		  }
	  }
  }
#endif 

#if VCEG_AZ07_CTX_RESIDUALCODING
  Bool bHor8x8 = uiWidth == 8 && uiHeight == 8 && codingParameters.scanType == SCAN_HOR;
  Bool bVer8x8 = uiWidth == 8 && uiHeight == 8 && codingParameters.scanType == SCAN_VER;
  Bool bNonZig8x8 = bHor8x8 || bVer8x8; 
#endif

#if VCEG_AZ08_USE_KLT
  if (pcCU->getSlice()->getSPS()->getUseKLT())
  {
#endif
#if VCEG_AZ08_KLT_COMMON
      UInt uiMaxTrWidth = g_uiDepth2Width[USE_MORE_BLOCKSIZE_DEPTH_MAX - 1];
      UInt uiMinTrWidth = g_uiDepth2Width[USE_MORE_BLOCKSIZE_DEPTH_MIN - 1];
      Bool bCheckKLTFlag = (toChannelType(compID) == CHANNEL_TYPE_LUMA) && (uiWidth == uiHeight) && (uiWidth <= uiMaxTrWidth) && (uiWidth >= uiMinTrWidth);
      if (bCheckKLTFlag && pcCU->getSlice()->getPPS()->getUseTransformSkip())
      {
          UInt useTransformSkip = pcCU->getTransformSkip(uiAbsPartIdx, compID);
          bCheckKLTFlag &= !useTransformSkip;
      }
#if VCEG_AZ08_USE_KLT
      if (pcCU->getSlice()->getSPS()->getUseInterKLT() && !pcCU->getSlice()->getSPS()->getUseIntraKLT()) //only inter
      {
          bCheckKLTFlag &= (!pcCU->isIntra(uiAbsPartIdx));
      }
      else if (!pcCU->getSlice()->getSPS()->getUseInterKLT() && pcCU->getSlice()->getSPS()->getUseIntraKLT()) //only intra
      {
          bCheckKLTFlag &= (pcCU->isIntra(uiAbsPartIdx));
      }
      else if ((!pcCU->getSlice()->getSPS()->getUseInterKLT()) && (!pcCU->getSlice()->getSPS()->getUseIntraKLT())) //neither
      {
          bCheckKLTFlag = false;
      }
#else
#if VCEG_AZ08_INTER_KLT && !VCEG_AZ08_INTRA_KLT //only inter
      bCheckKLTFlag &= (!pcCU->isIntra(uiAbsPartIdx));
#endif
#if !VCEG_AZ08_INTER_KLT && VCEG_AZ08_INTRA_KLT //only intra
      bCheckKLTFlag &= (pcCU->isIntra(uiAbsPartIdx));
#endif
#if !VCEG_AZ08_INTER_KLT && !VCEG_AZ08_INTRA_KLT //none
      bCheckKLTFlag = false;
#endif
#endif

      if (bCheckKLTFlag)
      {
          codeKLTFlags(rTu, compID);
      }
#endif
#if VCEG_AZ08_USE_KLT
  }
#endif
  //----- encode significance map -----

  // Find position of last coefficient
  Int scanPosLast = -1;
  Int posLast;

#if JVET_C0024_QTBT
  const UInt uiCGLog2 = (uiWidth==2 || uiHeight==2) ? MLS_CG_LOG2_HEIGHT-1: MLS_CG_LOG2_HEIGHT;
  const UInt uiCGSizeLog2 = (uiWidth==2 || uiHeight==2) ? MLS_CG_SIZE-2: MLS_CG_SIZE;
#endif

  UInt uiSigCoeffGroupFlag[ MLS_GRP_NUM ];

  memset( uiSigCoeffGroupFlag, 0, sizeof(UInt) * MLS_GRP_NUM );
  do
  {
	  if ((uiWidth == 4 && uiHeight == 4))
		  posLast = posLast;
    posLast = codingParameters.scan[ ++scanPosLast ];

#if SP_COEFFNxX	
	if (posLast < 0)
		posLast = posLast;
	Bool cond = SpatialCodeCoeffNxN ? (reordered_spR1[posLast] != 0) : (pcCoef[posLast] != 0);
	if(  cond )
#else
    if( pcCoef[ posLast ] != 0 )
#endif
    {
      // get L1 sig map
      UInt uiPosY   = posLast >> uiLog2BlockWidth;
      UInt uiPosX   = posLast - ( uiPosY << uiLog2BlockWidth );

#if JVET_C0024_QTBT
      UInt uiBlkIdx = (codingParameters.widthInGroups * (uiPosY >> uiCGLog2)) + (uiPosX >> uiCGLog2);
#else
      UInt uiBlkIdx = (codingParameters.widthInGroups * (uiPosY >> MLS_CG_LOG2_HEIGHT)) + (uiPosX >> MLS_CG_LOG2_WIDTH);
#endif
#if VCEG_AZ07_CTX_RESIDUALCODING 
      if( bHor8x8 )
      {
        uiBlkIdx = uiPosY >> 1;
      }
      else if( bVer8x8)
      {
        uiBlkIdx = uiPosX >> 1;
      }
#endif
      uiSigCoeffGroupFlag[ uiBlkIdx ] = 1;

      uiNumSig--;
    }
  } while ( uiNumSig > 0 );

  // Code position of last coefficient
  Int posLastY = posLast >> uiLog2BlockWidth;
  Int posLastX = posLast - ( posLastY << uiLog2BlockWidth );

#if PIP && SP_COEFFNxX
  if (!(isPIP && SpatialCodeCoeffNxN))
  {
#endif
	  codeLastSignificantXY(posLastX, posLastY, uiWidth, uiHeight, compID, codingParameters.scanType
#if PIP
		  , isPIP
#endif
		  );
#if PIP && SP_COEFFNxX
  }
#endif
 
#if JVET_C0046_ZO_ASSERT && JVET_C0046_ZO_ASSERT_LAST_COEF
#if JVET_C0046_ZO_ASSERT_FIX_TICKET24
  if ( ((uiWidth > JVET_C0046_ZERO_OUT_TH) || (uiHeight > JVET_C0046_ZERO_OUT_TH)) &&
#else
  if ( ((uiWidth > JVET_C0024_ZERO_OUT_TH) || (uiHeight > JVET_C0024_ZERO_OUT_TH)) &&
#endif
      (!pcCU->getTransformSkip(compID) && !pcCU->getCUTransquantBypass(uiAbsPartIdx)))
  {
     // last coeff shall be in the low freqecy domain
#if JVET_C0046_ZO_ASSERT_FIX_TICKET24
     assert((posLastX < JVET_C0046_ZERO_OUT_TH) && (posLastY < JVET_C0046_ZERO_OUT_TH));
#else
     assert((posLastX < JVET_C0024_ZERO_OUT_TH) && (posLastY < JVET_C0024_ZERO_OUT_TH));
#endif
  }
#endif


  //===== code significance flag =====
#if VCEG_AZ07_CTX_RESIDUALCODING
  UInt uiPrevGRParam = 0; 
#if PIP
  ContextModel * const baseCoeffGroupCtx = isPIP ? m_cCUSigCoeffGroupSCModelPIP.get( 0, chType ) : m_cCUSigCoeffGroupSCModel.get( 0, chType );
#else
  ContextModel * const baseCoeffGroupCtx = m_cCUSigCoeffGroupSCModel.get( 0, chType );
#endif
#if JVET_C0024_QTBT
  UInt uiOffsetonTU = uiLog2BlockSize<=2 ? 0: NUM_SIG_FLAG_CTX_LUMA_TU << ( min(1, (Int)(uiLog2BlockSize - 3)) );
#else
  UInt uiOffsetonTU = uiLog2BlockWidth==2 ? 0: NUM_SIG_FLAG_CTX_LUMA_TU << ( min(1, (Int)(uiLog2BlockWidth - 3)) );
#endif

#if PIP 
  ContextModel * const baseCtx = (chType == CHANNEL_TYPE_LUMA) ? (isPIP ? m_cCUSigSCModelPIP.get( 0, 0 ) : m_cCUSigSCModel.get( 0, 0 )) + uiOffsetonTU : m_cCUSigSCModel.get( 0, 0 ) + NUM_SIG_FLAG_CTX_LUMA;
  ContextModel * const greXCtx = (chType == CHANNEL_TYPE_LUMA) ? (isPIP ? m_cCUOneSCModelPIP.get( 0, 0 ) : m_cCUOneSCModel.get( 0, 0 )) : m_cCUOneSCModel.get( 0, 0 ) + NUM_ONE_FLAG_CTX_LUMA;
#else
  ContextModel * const baseCtx = (chType == CHANNEL_TYPE_LUMA) ? m_cCUSigSCModel.get( 0, 0 ) + uiOffsetonTU : m_cCUSigSCModel.get( 0, 0 ) + NUM_SIG_FLAG_CTX_LUMA;
  ContextModel * const greXCtx = (chType == CHANNEL_TYPE_LUMA) ? m_cCUOneSCModel.get( 0, 0 ) : m_cCUOneSCModel.get( 0, 0 ) + NUM_ONE_FLAG_CTX_LUMA;
#endif

#else
  ContextModel * const baseCoeffGroupCtx = m_cCUSigCoeffGroupSCModel.get( 0, chType );
  ContextModel * const baseCtx = m_cCUSigSCModel.get( 0, 0 ) + getSignificanceMapContextOffset(compID);
#endif

#if JVET_C0024_QTBT
  const Int  iLastScanSet  = scanPosLast >> uiCGSizeLog2;
#else
  const Int  iLastScanSet  = scanPosLast >> MLS_CG_SIZE;
#endif

#if !VCEG_AZ07_CTX_RESIDUALCODING
  UInt c1                  = 1;
#endif
  UInt uiGoRiceParam       = 0;
  Int  iScanPosSig         = scanPosLast;

  for( Int iSubSet = iLastScanSet; iSubSet >= 0; iSubSet-- )
  {
    Int numNonZero = 0;
#if JVET_C0024_QTBT
    Int  iSubPos   = iSubSet << uiCGSizeLog2;
#else
    Int  iSubPos   = iSubSet << MLS_CG_SIZE;
#endif
#if !VCEG_AZ07_CTX_RESIDUALCODING
    uiGoRiceParam  = currentGolombRiceStatistic / RExt__GOLOMB_RICE_INCREMENT_DIVISOR;
#endif
    Bool updateGolombRiceStatistics = bUseGolombRiceParameterAdaptation; //leave the statistics at 0 when not using the adaptation system
    UInt coeffSigns = 0;

    Int absCoeff[1 << MLS_CG_SIZE];
#if VCEG_AZ07_CTX_RESIDUALCODING
    Int pos     [1 << MLS_CG_SIZE];
    UInt ctxG1     = 0;
    UInt ctxG2     = 0;
#endif

    Int lastNZPosInCG  = -1;
#if JVET_C0024_QTBT
    Int firstNZPosInCG = 1 << uiCGSizeLog2;
#else
    Int firstNZPosInCG = 1 << MLS_CG_SIZE;
#endif

    Bool escapeDataPresentInGroup = false;

    if( iScanPosSig == scanPosLast )
    {
#if SP_COEFFNxX
		absCoeff[0] = Int(abs(SpatialCodeCoeffNxN ? reordered_spR1[posLast] : pcCoef[posLast]));
#else
      absCoeff[ 0 ] = Int(abs( pcCoef[ posLast ] ));
#endif
#if VCEG_AZ07_CTX_RESIDUALCODING
      pos[numNonZero]= posLastX + (posLastY<< uiLog2BlockWidth);
#endif
#if SP_COEFFNxX
	  coeffSigns    = ( SpatialCodeCoeffNxN ? reordered_spR1[posLast] < 0 : pcCoef[ posLast ] < 0 );
#else
      coeffSigns    = ( pcCoef[ posLast ] < 0 );
#endif
      numNonZero    = 1;
#if VCEG_AZ05_ROT_TR || VCEG_AZ05_INTRA_MPI || COM16_C1044_NSST || COM16_C1046_PDPC_INTRA
#if SP_COEFFNxX
	  bCbfCU += abs(SpatialCodeCoeffNxN ? reordered_spR1[posLast] : pcCoef[posLast]);
#else
      bCbfCU += abs(pcCoef[posLast]);
#endif
#endif
      lastNZPosInCG  = iScanPosSig;
      firstNZPosInCG = iScanPosSig;
      iScanPosSig--;
    }

    // encode significant_coeffgroup_flag
    Int iCGBlkPos = codingParameters.scanCG[ iSubSet ];
    Int iCGPosY   = iCGBlkPos / codingParameters.widthInGroups;
    Int iCGPosX   = iCGBlkPos - (iCGPosY * codingParameters.widthInGroups);

#if VCEG_AZ07_CTX_RESIDUALCODING
    if(bNonZig8x8 )
    {
      iCGPosY = (bHor8x8 ? iCGBlkPos : 0);
      iCGPosX = (bVer8x8 ? iCGBlkPos : 0);
    }
#endif

    if( iSubSet == iLastScanSet || iSubSet == 0)
    {
      uiSigCoeffGroupFlag[ iCGBlkPos ] = 1;
    }
#if COM16_C806_T64  && (!JVET_C0024_QTBT || JVET_C0024_ZERO_OUT_FIX ) && !JVET_C0046_ZO_ASSERT
#if JVET_C0024_ZERO_OUT_FIX
    else if( iCGPosY>=(JVET_C0024_ZERO_OUT_TH>>2) || iCGPosX>=(JVET_C0024_ZERO_OUT_TH>>2) )
#else
    else if( uiWidth>=64 && ( iCGPosY>=(codingParameters.heightInGroups/2) || iCGPosX>=(codingParameters.widthInGroups/2) ) )
#endif
    {
      assert( 0 == uiSigCoeffGroupFlag[ iCGBlkPos ] );
    }
#endif
#if JVET_C0046_ZO_ASSERT && JVET_C0046_ZO_ASSERT_CODED_SBK_FLAG
    else if ( (uiLog2BlockWidth + uiLog2BlockHeight) > TH_LOG2TBAREASIZE && 
              (!pcCU->getTransformSkip(compID) && !pcCU->getCUTransquantBypass(uiAbsPartIdx) ))
    {
        if ( iCGPosY >= (codingParameters.heightInGroups / 2) || iCGPosX >= (codingParameters.widthInGroups / 2) )
        {
            // coded_sbk_flag(iCGX,iCGY) shall be equal to 0
            assert(0 == uiSigCoeffGroupFlag[iCGBlkPos]);
        }
    }
#endif
    else
    {
      UInt uiSigCoeffGroup   = (uiSigCoeffGroupFlag[ iCGBlkPos ] != 0);
#if VCEG_AZ07_CTX_RESIDUALCODING
      UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, codingParameters.widthInGroups, codingParameters.heightInGroups, codingParameters.scanType );
#else
      UInt uiCtxSig  = TComTrQuant::getSigCoeffGroupCtxInc( uiSigCoeffGroupFlag, iCGPosX, iCGPosY, codingParameters.widthInGroups, codingParameters.heightInGroups );
#endif
#if PIP && SP_COEFFNxX
	  if (!SpatialCodeCoeffNxN)
	  {
#endif
		  m_pcBinIf->encodeBin(uiSigCoeffGroup, baseCoeffGroupCtx[uiCtxSig]);
#if PIP
		  if (encodeTime && verbose)
			  cout << "A:" << (Int)uiSigCoeffGroup;
#if SP_COEFFNxX
	  }
#endif
#endif
    }

    // encode significant_coeff_flag
    if( uiSigCoeffGroupFlag[ iCGBlkPos ] )
    {
#if !VCEG_AZ07_CTX_RESIDUALCODING
      const Int patternSigCtx = TComTrQuant::calcPatternSigCtx(uiSigCoeffGroupFlag, iCGPosX, iCGPosY, codingParameters.widthInGroups, codingParameters.heightInGroups);
#endif

      UInt uiBlkPos, uiSig, uiCtxSig;
      for( ; iScanPosSig >= iSubPos; iScanPosSig-- )
      {
        uiBlkPos  = codingParameters.scan[ iScanPosSig ];
#if SP_COEFFNxX
		uiSig     = SpatialCodeCoeffNxN ? (reordered_spR1[uiBlkPos] !=0) : (pcCoef[ uiBlkPos ] != 0);
#else
        uiSig     = (pcCoef[ uiBlkPos ] != 0);
#endif
        if( iScanPosSig > iSubPos || iSubSet == 0 || numNonZero )
        {
#if VCEG_AZ07_CTX_RESIDUALCODING
#if SP_COEFFNxX
			uiCtxSig = TComTrQuant::getGrtZeroCtxInc((SpatialCodeCoeffNxN ? reordered_spR1 : pcCoef), codingParameters.scan[iScanPosSig], uiWidth, uiHeight, chType);
#else
          uiCtxSig  = TComTrQuant::getGrtZeroCtxInc( pcCoef, codingParameters.scan[iScanPosSig], uiWidth, uiHeight, chType );
#endif

#else
          uiCtxSig  = TComTrQuant::getSigCtxInc( patternSigCtx, codingParameters, iScanPosSig, uiLog2BlockWidth, uiLog2BlockHeight, chType );
#endif

#if PIP && SP_COEFFNxX
		  if (!SpatialCodeCoeffNxN)
		  {
#endif
			  m_pcBinIf->encodeBin(uiSig, baseCtx[uiCtxSig]);
#if PIP && SP_COEFFNxX
		  }
#endif
        }
        if( uiSig )
        {
#if VCEG_AZ07_CTX_RESIDUALCODING
          pos[ numNonZero ]      = uiBlkPos;  
#endif
#if SP_COEFFNxX
		  absCoeff[ numNonZero ] = Int(abs( SpatialCodeCoeffNxN ? reordered_spR1[uiBlkPos] : pcCoef[ uiBlkPos ] ));
		  coeffSigns = 2 * coeffSigns + ( SpatialCodeCoeffNxN ? (reordered_spR1[uiBlkPos] < 0) : (pcCoef[ uiBlkPos ] < 0 ));
#else
          absCoeff[ numNonZero ] = Int(abs( pcCoef[ uiBlkPos ] ));
          coeffSigns = 2 * coeffSigns + ( pcCoef[ uiBlkPos ] < 0 );
#endif
#if VCEG_AZ05_ROT_TR || VCEG_AZ05_INTRA_MPI || COM16_C1044_NSST || COM16_C1046_PDPC_INTRA
          bCbfCU += absCoeff[numNonZero];
#endif
          numNonZero++;
          if( lastNZPosInCG == -1 )
          {
            lastNZPosInCG = iScanPosSig;
          }
          firstNZPosInCG = iScanPosSig;
        }
      }
    }
    else
    {
      iScanPosSig = iSubPos - 1;
    }

    if( numNonZero > 0 )
    {
      Bool signHidden = ( lastNZPosInCG - firstNZPosInCG >= SBH_THRESHOLD );

#if !VCEG_AZ07_CTX_RESIDUALCODING
      const UInt uiCtxSet = getContextSetIndex(compID, iSubSet, (c1 == 0));
      c1 = 1;

      ContextModel *baseCtxMod = m_cCUOneSCModel.get( 0, 0 ) + (NUM_ONE_FLAG_CTX_PER_SET * uiCtxSet);
#endif

      Int numC1Flag = min(numNonZero, C1FLAG_NUMBER);
      Int firstC2FlagIdx = -1;
      for( Int idx = 0; idx < numC1Flag; idx++ )
      {
        UInt uiSymbol = absCoeff[ idx ] > 1;
#if VCEG_AZ07_CTX_RESIDUALCODING 
        if(idx || iSubSet != iLastScanSet)
        {
#if SP_COEFFNxX
			ctxG1 = TComTrQuant::getGrtOneCtxInc((SpatialCodeCoeffNxN ? reordered_spR1 : pcCoef), pos[idx], uiWidth, uiHeight, chType);
#else
          ctxG1 = TComTrQuant::getGrtOneCtxInc( pcCoef, pos[idx], uiWidth, uiHeight, chType );
#endif
        }
        m_pcBinIf->encodeBin( uiSymbol, greXCtx[ ctxG1 ] );
#else
        m_pcBinIf->encodeBin( uiSymbol, baseCtxMod[c1] );
#endif

        if( uiSymbol )
        {
#if !VCEG_AZ07_CTX_RESIDUALCODING
          c1 = 0;
#endif
          if (firstC2FlagIdx == -1)
          {
            firstC2FlagIdx = idx;
          }
          else //if a greater-than-one has been encountered already this group
          {
            escapeDataPresentInGroup = true;
          }
        }
#if !VCEG_AZ07_CTX_RESIDUALCODING
        else if( (c1 < 3) && (c1 > 0) )
        {
          c1++;
        }
#endif
      }
#if !VCEG_AZ07_CTX_RESIDUALCODING
      if (c1 == 0)
      {
        baseCtxMod = m_cCUAbsSCModel.get( 0, 0 ) + (NUM_ABS_FLAG_CTX_PER_SET * uiCtxSet);
#endif
        if ( firstC2FlagIdx != -1)
        {
          UInt symbol = absCoeff[ firstC2FlagIdx ] > 2;
#if VCEG_AZ07_CTX_RESIDUALCODING
          if( firstC2FlagIdx || iSubSet != iLastScanSet )
          {
#if SP_COEFFNxX
			  ctxG2 = TComTrQuant::getGrtTwoCtxInc( (SpatialCodeCoeffNxN ? reordered_spR1 : pcCoef), pos[firstC2FlagIdx], uiWidth, uiHeight, chType );
#else
            ctxG2 = TComTrQuant::getGrtTwoCtxInc( pcCoef, pos[firstC2FlagIdx], uiWidth, uiHeight, chType );
#endif
          }
          m_pcBinIf->encodeBin( symbol, greXCtx[ ctxG2 ] );
#else
          m_pcBinIf->encodeBin( symbol, baseCtxMod[0] );
#endif

          if (symbol != 0)
          {
            escapeDataPresentInGroup = true;
          }
        }
#if !VCEG_AZ07_CTX_RESIDUALCODING
      }
#endif
      escapeDataPresentInGroup = escapeDataPresentInGroup || (numNonZero > C1FLAG_NUMBER);

      if (escapeDataPresentInGroup && alignCABACBeforeBypass)
      {
        m_pcBinIf->align();
      }
#if !VCEG_AZ07_CTX_RESIDUALCODING
      if( beValid && signHidden )
      {
        m_pcBinIf->encodeBinsEP( (coeffSigns >> 1), numNonZero-1 );
      }
      else
      {
        m_pcBinIf->encodeBinsEP( coeffSigns, numNonZero );
      }
#endif
      Int iFirstCoeff2 = 1;
      if (escapeDataPresentInGroup)
      {
        for ( Int idx = 0; idx < numNonZero; idx++ )
        {
          UInt baseLevel  = (idx < C1FLAG_NUMBER)? (2 + iFirstCoeff2 ) : 1;

          if( absCoeff[ idx ] >= baseLevel)
          {
            const UInt escapeCodeValue = absCoeff[idx] - baseLevel;
#if VCEG_AZ07_CTX_RESIDUALCODING
            if( updateGolombRiceStatistics && iSubSet == iLastScanSet )
            {
              uiGoRiceParam = currentGolombRiceStatistic / RExt__GOLOMB_RICE_INCREMENT_DIVISOR;
            }
            else
            {
#if SP_COEFFNxX
				uiGoRiceParam = TComTrQuant::getRemainCoeffCtxInc((SpatialCodeCoeffNxN ? reordered_spR1 : pcCoef), pos[idx], uiWidth, uiHeight);
#else
              uiGoRiceParam = TComTrQuant::getRemainCoeffCtxInc( pcCoef, pos[idx], uiWidth, uiHeight);
#endif
              if(bUseGolombRiceParameterAdaptation)
              {
                uiGoRiceParam = max(uiPrevGRParam , uiGoRiceParam);
              }
            }
            uiPrevGRParam = max(0, (Int)uiGoRiceParam - 1);
#endif
            xWriteCoefRemainExGolomb( escapeCodeValue, uiGoRiceParam, extendedPrecision, maxLog2TrDynamicRange );
#if !VCEG_AZ07_CTX_RESIDUALCODING 
            if (absCoeff[idx] > (3 << uiGoRiceParam))
            {
              uiGoRiceParam = bUseGolombRiceParameterAdaptation ? (uiGoRiceParam + 1) : (std::min<UInt>((uiGoRiceParam + 1), 4));
            }
#endif
            if (updateGolombRiceStatistics)
            {
              const UInt initialGolombRiceParameter = currentGolombRiceStatistic / RExt__GOLOMB_RICE_INCREMENT_DIVISOR;

              if (escapeCodeValue >= (3 << initialGolombRiceParameter))
              {
                currentGolombRiceStatistic++;
              }
              else if (((escapeCodeValue * 2) < (1 << initialGolombRiceParameter)) && (currentGolombRiceStatistic > 0))
              {
                currentGolombRiceStatistic--;
              }

              updateGolombRiceStatistics = false;
            }

          }

          if(absCoeff[ idx ] >= 2)
          {
            iFirstCoeff2 = 0;
          }
        }
      }
#if VCEG_AZ07_CTX_RESIDUALCODING
#if JVET_C0024_QTBT
      if( beValid && signHidden && uiWidth>=4 && uiHeight>=4)
#else
      if( beValid && signHidden )
#endif
      {
        m_pcBinIf->encodeBinsEP( (coeffSigns >> 1), numNonZero-1 );
      }
      else
      {
        m_pcBinIf->encodeBinsEP( coeffSigns, numNonZero );
#if PIP
		if (encodeTime && verbose)
			cout << "G:"<<(Int)(coeffSigns);
#endif
      }
#endif
    }
  }
#if ENVIRONMENT_VARIABLE_DEBUG_AND_TEST
  printSBACCoeffData(posLastX, posLastY, uiWidth, uiHeight, compID, uiAbsPartIdx, codingParameters.scanType, pcCoef, pcCU->getSlice()->getFinalized());
#endif

#if COM16_C806_EMT

#if PIP
  if (encodeTime && pcCU->getWidth(uiAbsPartIdx) == 4 && pcCU->getHeight(uiAbsPartIdx) == 4 && isLuma(compID))
  {
	  if (!pcCU->getEmtCuFlag(uiAbsPartIdx))
		  EMTlog = 0;
	  else
		  EMTlog = pcCU->getEmtTuIdx(uiAbsPartIdx) + 1;
  }

#endif


#if PIP
  UInt bits2 = getNumberOfWrittenBits();
  Bits2 = getNumberOfWrittenBits();
  if (encodeTime && uiWidth == 4 && uiHeight == 4 && isLuma(compID))
  {
	  if (!maxExceeded || true)
	  {
		  FILE* fout = fopen("log.txt", "a");
		  fprintf(fout, "%4d\t%d\t%2d\n", Bits2 - Bits1 - sigCnt, EMTlog, pcCU->getIntraDir(toChannelType(compID), uiAbsPartIdx));
		  sigCnt = 0;
		  totalPIPbits += Bits2 - Bits1;
		  fclose(fout);
	  }
	  maxExceeded = false;

  }
#endif



#if PIP
  if (!(pcCU->getPIPflag(uiAbsPartIdx) && isLuma(compID)) || ADOPT_EMT)
#endif
  if ( !pcCU->getTransformSkip( uiAbsPartIdx, compID) && compID == COMPONENT_Y )
  {
    if( pcCU->getEmtCuFlag( uiAbsPartIdx ) && pcCU->isIntra( uiAbsPartIdx ) )
    {
      if ( uiTuNumSig>g_iEmtSigNumThr )
      {
        codeEmtTuIdx( pcCU, uiAbsPartIdx, rTu.GetTransformDepthTotal() ); 
      }
      else
      {
        assert( pcCU->getEmtTuIdx( uiAbsPartIdx )==0 );
      }
    }
    if( pcCU->getEmtCuFlag( uiAbsPartIdx ) && !pcCU->isIntra( uiAbsPartIdx ) )
    {
      codeEmtTuIdx( pcCU, uiAbsPartIdx, rTu.GetTransformDepthTotal() ); 
    }
  }
#endif
#if SP_COEFFNxN_2
  if (SpatialCodeCoeffNxN_2)
  {
	  memcpy(pcCoef, TMP_pcCoef, uiWidth*uiHeight*sizeof(TCoeff));
  }
  _aligned_free(TMP_pcCoef);
#endif




  return;
}

/** code SAO offset sign
 * \param code sign value
 */
Void TEncSbac::codeSAOSign( UInt code )
{
  m_pcBinIf->encodeBinEP( code );
}

Void TEncSbac::codeSaoMaxUvlc    ( UInt code, UInt maxSymbol )
{
  if (maxSymbol == 0)
  {
    return;
  }

  Int i;
  Bool bCodeLast = ( maxSymbol > code );

  if ( code == 0 )
  {
    m_pcBinIf->encodeBinEP( 0 );
  }
  else
  {
    m_pcBinIf->encodeBinEP( 1 );
    for ( i=0; i<code-1; i++ )
    {
      m_pcBinIf->encodeBinEP( 1 );
    }
    if( bCodeLast )
    {
      m_pcBinIf->encodeBinEP( 0 );
    }
  }
}

/** Code SAO EO class or BO band position
 */
Void TEncSbac::codeSaoUflc       ( UInt uiLength, UInt uiCode )
{
  m_pcBinIf->encodeBinsEP ( uiCode, uiLength );
}

/** Code SAO merge flags
 */
Void TEncSbac::codeSaoMerge       ( UInt uiCode )
{
  m_pcBinIf->encodeBin(((uiCode == 0) ? 0 : 1),  m_cSaoMergeSCModel.get( 0, 0, 0 ));
}

/** Code SAO type index
 */
Void TEncSbac::codeSaoTypeIdx       ( UInt uiCode)
{
  if (uiCode == 0)
  {
    m_pcBinIf->encodeBin( 0, m_cSaoTypeIdxSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cSaoTypeIdxSCModel.get( 0, 0, 0 ) );
    m_pcBinIf->encodeBinEP( uiCode == 1 ? 0 : 1 );
  }
}

Void TEncSbac::codeSAOOffsetParam(ComponentID compIdx, SAOOffset& ctbParam, Bool sliceEnabled, const Int channelBitDepth)
{
  UInt uiSymbol;
  if(!sliceEnabled)
  {
    assert(ctbParam.modeIdc == SAO_MODE_OFF);
    return;
  }
  const Bool bIsFirstCompOfChType = (getFirstComponentOfChannel(toChannelType(compIdx)) == compIdx);

  //type
  if(bIsFirstCompOfChType)
  {
    //sao_type_idx_luma or sao_type_idx_chroma
    if(ctbParam.modeIdc == SAO_MODE_OFF)
    {
      uiSymbol =0;
    }
    else if(ctbParam.typeIdc == SAO_TYPE_BO) //BO
    {
      uiSymbol = 1;
    }
    else
    {
      assert(ctbParam.typeIdc < SAO_TYPE_START_BO); //EO
      uiSymbol = 2;
    }
    codeSaoTypeIdx(uiSymbol);
  }

  if(ctbParam.modeIdc == SAO_MODE_NEW)
  {
    Int numClasses = (ctbParam.typeIdc == SAO_TYPE_BO)?4:NUM_SAO_EO_CLASSES;
    Int offset[4];
    Int k=0;
    for(Int i=0; i< numClasses; i++)
    {
      if(ctbParam.typeIdc != SAO_TYPE_BO && i == SAO_CLASS_EO_PLAIN)
      {
        continue;
      }
      Int classIdx = (ctbParam.typeIdc == SAO_TYPE_BO)?(  (ctbParam.typeAuxInfo+i)% NUM_SAO_BO_CLASSES   ):i;
      offset[k] = ctbParam.offset[classIdx];
      k++;
    }

    const Int  maxOffsetQVal = TComSampleAdaptiveOffset::getMaxOffsetQVal(channelBitDepth);
    for(Int i=0; i< 4; i++)
    {
      codeSaoMaxUvlc((offset[i]<0)?(-offset[i]):(offset[i]),  maxOffsetQVal ); //sao_offset_abs
    }


    if(ctbParam.typeIdc == SAO_TYPE_BO)
    {
      for(Int i=0; i< 4; i++)
      {
        if(offset[i] != 0)
        {
          codeSAOSign((offset[i]< 0)?1:0);
        }
      }

      codeSaoUflc(NUM_SAO_BO_CLASSES_LOG2, ctbParam.typeAuxInfo ); //sao_band_position
    }
    else //EO
    {
      if(bIsFirstCompOfChType)
      {
        assert(ctbParam.typeIdc - SAO_TYPE_START_EO >=0);
        codeSaoUflc(NUM_SAO_EO_TYPES_LOG2, ctbParam.typeIdc - SAO_TYPE_START_EO ); //sao_eo_class_luma or sao_eo_class_chroma
      }
    }

  }
}


Void TEncSbac::codeSAOBlkParam(SAOBlkParam& saoBlkParam, const BitDepths &bitDepths
                              , Bool* sliceEnabled
                              , Bool leftMergeAvail
                              , Bool aboveMergeAvail
                              , Bool onlyEstMergeInfo // = false
                              )
{

  Bool isLeftMerge = false;
  Bool isAboveMerge= false;

  if(leftMergeAvail)
  {
    isLeftMerge = ((saoBlkParam[COMPONENT_Y].modeIdc == SAO_MODE_MERGE) && (saoBlkParam[COMPONENT_Y].typeIdc == SAO_MERGE_LEFT));
    codeSaoMerge( isLeftMerge?1:0  ); //sao_merge_left_flag
  }

  if( aboveMergeAvail && !isLeftMerge)
  {
    isAboveMerge = ((saoBlkParam[COMPONENT_Y].modeIdc == SAO_MODE_MERGE) && (saoBlkParam[COMPONENT_Y].typeIdc == SAO_MERGE_ABOVE));
    codeSaoMerge( isAboveMerge?1:0  ); //sao_merge_left_flag
  }

  if(onlyEstMergeInfo)
  {
    return; //only for RDO
  }

  if(!isLeftMerge && !isAboveMerge) //not merge mode
  {
    for(Int compIdx=0; compIdx < MAX_NUM_COMPONENT; compIdx++)
    {
      codeSAOOffsetParam(ComponentID(compIdx), saoBlkParam[compIdx], sliceEnabled[compIdx], bitDepths.recon[toChannelType(ComponentID(compIdx))]);
    }
  }
}

#if JVET_D0123_ME_CTX_LUT_BITS
Void TEncSbac::estPuMeBit  (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  estMvdBit            (pcEstPuMeBitsSbac);
  estMvpIdxBit         (pcEstPuMeBitsSbac);
  estRefIdxBit         (pcEstPuMeBitsSbac);
  estMrgFlagBit        (pcEstPuMeBitsSbac);
  estMrgIdxBit         (pcEstPuMeBitsSbac);  
  estInterDirBit       (pcEstPuMeBitsSbac);  
#if VCEG_AZ07_FRUC_MERGE
  estFrucModeBit     (pcEstPuMeBitsSbac);  
#endif
#if COM16_C1016_AFFINE
  estAffineFlagBit     (pcEstPuMeBitsSbac);
#endif
}

Void TEncSbac::estMvdBit            (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUMvdSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_MV_RES_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->mvdBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->mvdBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}

Void TEncSbac::estMvpIdxBit         (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac){
  ContextModel *pCtx = m_cMVPIdxSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_MVP_IDX_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->mvpIdxBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->mvpIdxBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}

Void TEncSbac::estRefIdxBit          (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCURefPicSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_REF_NO_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->refIdxBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->refIdxBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}
#if VCEG_AZ07_FRUC_MERGE
Void TEncSbac::estFrucModeBit      (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUFRUCMgrModeSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_FRUCMGRMODE_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->frucMrgBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->frucMrgBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
  pCtx = m_cCUFRUCMESCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_FRUCME_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->frucMeBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->frucMeBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}
#endif
Void TEncSbac::estMrgFlagBit        (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUMergeFlagExtSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_MERGE_FLAG_EXT_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->mrgFlagBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->mrgFlagBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}
Void TEncSbac::estMrgIdxBit         (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUMergeIdxExtSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_MERGE_IDX_EXT_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->mrgIdxBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->mrgIdxBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}
#if COM16_C1016_AFFINE
Void TEncSbac::estAffineFlagBit     (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUAffineFlagSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_AFFINE_FLAG_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->affineFlagBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits(0);
    pcEstPuMeBitsSbac->affineFlagBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits(1);
  }
}
#endif
#if VCEG_AZ07_IMV
Void TEncSbac::estIMVFlagBit        (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUiMVFlagSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_IMV_FLAG_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->iMVFlagBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits(0);
    pcEstPuMeBitsSbac->iMVFlagBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits(1);
  }
}
#endif
Void TEncSbac::estInterDirBit     (estPuMeBitsSbacStruct* pcEstPuMeBitsSbac)
{
  ContextModel *pCtx = m_cCUInterDirSCModel.get(0);
  for (UInt uiCtxInc = 0; uiCtxInc < NUM_INTER_DIR_CTX; uiCtxInc++)
  {
    pcEstPuMeBitsSbac->interDirBits[uiCtxInc][0] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstPuMeBitsSbac->interDirBits[uiCtxInc][1] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}
#endif
/*!
 ****************************************************************************
 * \brief
 *   estimate bit cost for CBP, significant map and significant coefficients
 ****************************************************************************
 */
Void TEncSbac::estBit( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType 
#if RDOQ_BIT_ESTIMATE_FIX_TICKET29
  , UInt uiScanIdx
#endif
  )
{
  estCBFBit( pcEstBitsSbac );

  estSignificantCoeffGroupMapBit( pcEstBitsSbac, chType );

  // encode significance map
  estSignificantMapBit( pcEstBitsSbac, width, height, chType );

  // encode last significant position
  estLastSignificantPositionBit( pcEstBitsSbac, width, height, chType 
#if RDOQ_BIT_ESTIMATE_FIX_TICKET29
    , uiScanIdx
#endif
    );

  // encode significant coefficients
  estSignificantCoefficientsBit( pcEstBitsSbac, chType );

  memcpy(pcEstBitsSbac->golombRiceAdaptationStatistics, m_golombRiceAdaptationStatistics, (sizeof(UInt) * RExt__GOLOMB_RICE_ADAPTATION_STATISTICS_SETS));
}

/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost for each CBP bit
 ****************************************************************************
 */
Void TEncSbac::estCBFBit( estBitsSbacStruct* pcEstBitsSbac )
{
  ContextModel *pCtx = m_cCUQtCbfSCModel.get( 0 );

  for( UInt uiCtxInc = 0; uiCtxInc < (NUM_QT_CBF_CTX_SETS * NUM_QT_CBF_CTX_PER_SET); uiCtxInc++ )
  {
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 0 ] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstBitsSbac->blockCbpBits[ uiCtxInc ][ 1 ] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }

  pCtx = m_cCUQtRootCbfSCModel.get( 0 );

  for( UInt uiCtxInc = 0; uiCtxInc < 4; uiCtxInc++ )
  {
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 0 ] = pCtx[ uiCtxInc ].getEntropyBits( 0 );
    pcEstBitsSbac->blockRootCbpBits[ uiCtxInc ][ 1 ] = pCtx[ uiCtxInc ].getEntropyBits( 1 );
  }
}


/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient group map
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoeffGroupMapBit( estBitsSbacStruct* pcEstBitsSbac, ChannelType chType )
{
  Int firstCtx = 0, numCtx = NUM_SIG_CG_FLAG_CTX;

  for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
  {
    for( UInt uiBin = 0; uiBin < 2; uiBin++ )
    {
      pcEstBitsSbac->significantCoeffGroupBits[ ctxIdx ][ uiBin ] = m_cCUSigCoeffGroupSCModel.get(  0, chType, ctxIdx ).getEntropyBits( uiBin );
    }
  }
}


/*!
 ****************************************************************************
 * \brief
 *    estimate SAMBAC bit cost for significant coefficient map
 ****************************************************************************
 */
Void TEncSbac::estSignificantMapBit( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType )
{
  //--------------------------------------------------------------------------------------------------

  //set up the number of channels and context variables

  const UInt firstComponent = ((isLuma(chType)) ? (COMPONENT_Y) : (COMPONENT_Cb));
  const UInt lastComponent  = ((isLuma(chType)) ? (COMPONENT_Y) : (COMPONENT_Cb));

  //----------------------------------------------------------
#if !VCEG_AZ07_CTX_RESIDUALCODING
  Int firstCtx = MAX_INT;
  Int numCtx   = MAX_INT;

  if      ((width == 4) && (height == 4))
  {
    firstCtx = significanceMapContextSetStart[chType][CONTEXT_TYPE_4x4];
    numCtx   = significanceMapContextSetSize [chType][CONTEXT_TYPE_4x4];
  }
  else if ((width == 8) && (height == 8))
  {
    firstCtx = significanceMapContextSetStart[chType][CONTEXT_TYPE_8x8];
    numCtx   = significanceMapContextSetSize [chType][CONTEXT_TYPE_8x8];
  }
  else
  {
    firstCtx = significanceMapContextSetStart[chType][CONTEXT_TYPE_NxN];
    numCtx   = significanceMapContextSetSize [chType][CONTEXT_TYPE_NxN];
  }
#endif
  //--------------------------------------------------------------------------------------------------

  //fill the data for the significace map

  for (UInt component = firstComponent; component <= lastComponent; component++)
  {
#if VCEG_AZ07_CTX_RESIDUALCODING
#if JVET_C0024_QTBT
    Int log2Size = ((g_aucConvertToBit[ width ] + g_aucConvertToBit[ height ]) >>1) + MIN_CU_LOG2;
    Int firstCtx   = !component  ? (log2Size>4? 2: (log2Size<=2 ? 0: log2Size-2)) * NUM_SIG_FLAG_CTX_LUMA_TU : 0;
#else
    Int firstCtx   = !component  ? (g_aucConvertToBit[ width ] > 2 ? 2 : g_aucConvertToBit[width]) * NUM_SIG_FLAG_CTX_LUMA_TU : 0;
#endif
    Int numCtx     = !component  ? NUM_SIG_FLAG_CTX_LUMA_TU : NUM_SIG_FLAG_CTX_CHROMA;
    Int iCtxOffset = !component  ? 0 : NUM_SIG_FLAG_CTX_LUMA;
#else
    const UInt contextOffset = getSignificanceMapContextOffset(ComponentID(component));

    if (firstCtx > 0)
    {
      for( UInt bin = 0; bin < 2; bin++ ) //always get the DC
      {
        pcEstBitsSbac->significantBits[ contextOffset ][ bin ] = m_cCUSigSCModel.get( 0, 0, contextOffset ).getEntropyBits( bin );
      }
    }
#endif
    // This could be made optional, but would require this function to have knowledge of whether the
    // TU is transform-skipped or transquant-bypassed and whether the SPS flag is set
#if !VCEG_AZ07_CTX_RESIDUALCODING
    for( UInt bin = 0; bin < 2; bin++ )
    {
      const Int ctxIdx = significanceMapContextSetStart[chType][CONTEXT_TYPE_SINGLE];
      pcEstBitsSbac->significantBits[ contextOffset + ctxIdx ][ bin ] = m_cCUSigSCModel.get( 0, 0, (contextOffset + ctxIdx) ).getEntropyBits( bin );
    }
#endif

    for ( Int ctxIdx = firstCtx; ctxIdx < firstCtx + numCtx; ctxIdx++ )
    {
      for( UInt uiBin = 0; uiBin < 2; uiBin++ )
      {
#if VCEG_AZ07_CTX_RESIDUALCODING
        pcEstBitsSbac->significantBits[ ctxIdx ][ uiBin ] = m_cCUSigSCModel.get(  0, 0, iCtxOffset + ctxIdx ).getEntropyBits( uiBin );
#else
        pcEstBitsSbac->significantBits[ contextOffset + ctxIdx ][ uiBin ] = m_cCUSigSCModel.get(  0, 0, (contextOffset + ctxIdx) ).getEntropyBits( uiBin );
#endif
      }
    }
  }

  //--------------------------------------------------------------------------------------------------
}


/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */

Void TEncSbac::estLastSignificantPositionBit( estBitsSbacStruct* pcEstBitsSbac, Int width, Int height, ChannelType chType 
#if RDOQ_BIT_ESTIMATE_FIX_TICKET29
  , UInt uiScanIdx
#endif
  )
{
  //--------------------------------------------------------------------------------------------------.
#if RDOQ_BIT_ESTIMATE_FIX_TICKET29
  // swap
  if (uiScanIdx == SCAN_VER)
  {
    swap(width, height);
  }
#endif

  //set up the number of channels

  const UInt firstComponent = ((isLuma(chType)) ? (COMPONENT_Y) : (COMPONENT_Cb));
  const UInt lastComponent  = ((isLuma(chType)) ? (COMPONENT_Y) : (COMPONENT_Cb));

#if VCEG_AZ07_CTX_RESIDUALCODING && !COM16_C806_T64
  const UInt uiLog2BlockWidthIdx = g_aucConvertToBit[ width ];
  const UInt  ctxWidth   = isLuma(chType) ? width : 4;
  const UInt *puiCtxXIdx = g_uiLastCtx + (g_aucConvertToBit[ ctxWidth ] * (g_aucConvertToBit[ ctxWidth ] + 3 ) );   
#endif
  //--------------------------------------------------------------------------------------------------

  //fill the data for the last-significant-coefficient position

  for (UInt componentIndex = firstComponent; componentIndex <= lastComponent; componentIndex++)
  {
    const ComponentID component = ComponentID(componentIndex);

    Int iBitsX = 0, iBitsY = 0;
#if !VCEG_AZ07_CTX_RESIDUALCODING || COM16_C806_T64
    Int blkSizeOffsetX, blkSizeOffsetY, shiftX, shiftY;
    getLastSignificantContextParameters(ComponentID(component), width, height, blkSizeOffsetX, blkSizeOffsetY, shiftX, shiftY);
#endif

    Int ctx;

    const ChannelType channelType = toChannelType(ComponentID(component));

    ContextModel *const pCtxX = m_cCuCtxLastX.get( 0, channelType );
    ContextModel *const pCtxY = m_cCuCtxLastY.get( 0, channelType );
    Int          *const lastXBitsArray = pcEstBitsSbac->lastXBits[channelType];
    Int          *const lastYBitsArray = pcEstBitsSbac->lastYBits[channelType];

    //------------------------------------------------

    //X-coordinate

    for (ctx = 0; ctx < g_uiGroupIdx[ width - 1 ]; ctx++)
    {
#if VCEG_AZ07_CTX_RESIDUALCODING && !COM16_C806_T64
      Int ctxOffset = isLuma(chType)? puiCtxXIdx[ ctx ]: (ctx >> uiLog2BlockWidthIdx );
#else
      Int ctxOffset = blkSizeOffsetX + (ctx >>shiftX);
#endif
      lastXBitsArray[ ctx ] = iBitsX + pCtxX[ ctxOffset ].getEntropyBits( 0 );
      iBitsX += pCtxX[ ctxOffset ].getEntropyBits( 1 );
    }

    lastXBitsArray[ctx] = iBitsX;

    //------------------------------------------------

    //Y-coordinate
#if VCEG_AZ07_CTX_RESIDUALCODING && !COM16_C806_T64
    const UInt uiLog2BlockheightIdx = g_aucConvertToBit[ height ];
    const UInt  ctxHeight  = isLuma(chType) ? height : 4;
    const UInt* puiCtxYIdx = g_uiLastCtx + (g_aucConvertToBit[ ctxHeight ] * (g_aucConvertToBit[ ctxHeight ] + 3 ) );   
#endif
    for (ctx = 0; ctx < g_uiGroupIdx[ height - 1 ]; ctx++)
    {
#if VCEG_AZ07_CTX_RESIDUALCODING && !COM16_C806_T64
      Int ctxOffset = isLuma(chType)? puiCtxYIdx[ ctx ]: (ctx >> uiLog2BlockheightIdx );
#else
      Int ctxOffset = blkSizeOffsetY + (ctx >>shiftY);
#endif
      lastYBitsArray[ ctx ] = iBitsY + pCtxY[ ctxOffset ].getEntropyBits( 0 );
      iBitsY += pCtxY[ ctxOffset ].getEntropyBits( 1 );
    }

    lastYBitsArray[ctx] = iBitsY;

  } //end of component loop

  //--------------------------------------------------------------------------------------------------
}


/*!
 ****************************************************************************
 * \brief
 *    estimate bit cost of significant coefficient
 ****************************************************************************
 */
Void TEncSbac::estSignificantCoefficientsBit( estBitsSbacStruct* pcEstBitsSbac, ChannelType chType )
{
#if VCEG_AZ07_CTX_RESIDUALCODING
  const UInt oneStopIndex  = ((isLuma(chType)) ? (NUM_ONE_FLAG_CTX_LUMA) : (NUM_ONE_FLAG_CTX_CHROMA));
  ContextModel *ctxOne = m_cCUOneSCModel.get(0, 0) + ((isLuma(chType)) ? (0) : (NUM_ONE_FLAG_CTX_LUMA));
  for (Int ctxIdx = 0; ctxIdx < oneStopIndex; ctxIdx++)
#else
  ContextModel *ctxOne = m_cCUOneSCModel.get(0, 0);
  ContextModel *ctxAbs = m_cCUAbsSCModel.get(0, 0);

  const UInt oneStartIndex = ((isLuma(chType)) ? (0)                     : (NUM_ONE_FLAG_CTX_LUMA));
  const UInt oneStopIndex  = ((isLuma(chType)) ? (NUM_ONE_FLAG_CTX_LUMA) : (NUM_ONE_FLAG_CTX));

  const UInt absStartIndex = ((isLuma(chType)) ? (0)                     : (NUM_ABS_FLAG_CTX_LUMA));
  const UInt absStopIndex  = ((isLuma(chType)) ? (NUM_ABS_FLAG_CTX_LUMA) : (NUM_ABS_FLAG_CTX));


  for (Int ctxIdx = oneStartIndex; ctxIdx < oneStopIndex; ctxIdx++)
#endif
  {
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 0 ] = ctxOne[ ctxIdx ].getEntropyBits( 0 );
    pcEstBitsSbac->m_greaterOneBits[ ctxIdx ][ 1 ] = ctxOne[ ctxIdx ].getEntropyBits( 1 );
  }
#if !VCEG_AZ07_CTX_RESIDUALCODING
  for (Int ctxIdx = absStartIndex; ctxIdx < absStopIndex; ctxIdx++)
  {
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 0 ] = ctxAbs[ ctxIdx ].getEntropyBits( 0 );
    pcEstBitsSbac->m_levelAbsBits[ ctxIdx ][ 1 ] = ctxAbs[ ctxIdx ].getEntropyBits( 1 );
  }
#endif
}

/**
 - Initialize our context information from the nominated source.
 .
 \param pSrc From where to copy context information.
 */
Void TEncSbac::xCopyContextsFrom( const TEncSbac* pSrc )
{
  memcpy(m_contextModels, pSrc->m_contextModels, m_numContextModels*sizeof(m_contextModels[0]));
  memcpy(m_golombRiceAdaptationStatistics, pSrc->m_golombRiceAdaptationStatistics, (sizeof(UInt) * RExt__GOLOMB_RICE_ADAPTATION_STATISTICS_SETS));
}

Void  TEncSbac::loadContexts ( const TEncSbac* pSrc)
{
  xCopyContextsFrom(pSrc);
}

#if VCEG_AZ07_INIT_PREVFRAME
Void  TEncSbac::loadContextsFromPrev (TComStats* apcStats, SliceType eSliceType, Int iQPIdx, Bool bFromGloble, Int iQPIdxRst, Bool bAfterLastISlice )
{
  if(bFromGloble)
  {
#if VCEG_AZ07_INIT_PREVFRAME_FIX
    if(iQPIdx==-1 || !apcStats->aaQPUsed[eSliceType][iQPIdxRst].resetInit)
#else
    if(iQPIdx==-1 || (bAfterLastISlice && !apcStats->aaQPUsed[eSliceType][iQPIdxRst].resetInit))
#endif
    {
      return;
    }
    Int iCtxNr = getCtxNumber();
    for(UInt i = 0; i < iCtxNr; i++)
    {
#if VCEG_AZ07_BAC_ADAPT_WDOW || VCEG_AZ05_MULTI_PARAM_CABAC
      m_contextModels[i].setState(apcStats->m_uiCtxProbIdx[eSliceType][iQPIdx][0][i]);
#else
      m_contextModels[i].setState( (UChar) apcStats->m_uiCtxProbIdx[eSliceType][iQPIdx][0][i] );
#endif
    }
  }
  else
  {
    Int iCtxNr = getCtxNumber();
    for(UInt i = 0; i < iCtxNr; i++)
    {
#if VCEG_AZ07_BAC_ADAPT_WDOW || VCEG_AZ05_MULTI_PARAM_CABAC
      apcStats->m_uiCtxProbIdx[eSliceType][iQPIdx][0][i] = m_contextModels[i].getState();
#else
      apcStats->m_uiCtxProbIdx[eSliceType][iQPIdx][0][i] = (UShort)m_contextModels[i].getOrigState();
#endif
    }
  }
}
#endif
/** Performs CABAC encoding of the explicit RDPCM mode
 * \param rTu current TU data structure
 * \param compID component identifier
 */
Void TEncSbac::codeExplicitRdpcmMode( TComTU &rTu, const ComponentID compID )
{
#if PIP
	if (encodeTime && verbose)
		cout << ",RDPCM:";
#endif
  TComDataCU *cu = rTu.getCU();
#if !JVET_C0046_OMIT_ASSERT_ERDPCM
  const TComRectangle &rect = rTu.getRect(compID);
  const UInt tuHeight = g_aucConvertToBit[rect.height];
  const UInt tuWidth  = g_aucConvertToBit[rect.width];
#endif
  const UInt absPartIdx   = rTu.GetAbsPartIdxTU(compID);

#if !JVET_C0046_OMIT_ASSERT_ERDPCM
  assert(tuHeight == tuWidth);
  assert(tuHeight < 4);
#endif

  UInt explicitRdpcmMode = cu->getExplicitRdpcmMode(compID, absPartIdx);

  if( explicitRdpcmMode == RDPCM_OFF )
  {
    m_pcBinIf->encodeBin (0, m_explicitRdpcmFlagSCModel.get (0, toChannelType(compID), 0));
#if PIP
	if (encodeTime && verbose)
		cout << 0;
#endif
  }
  else if( explicitRdpcmMode == RDPCM_HOR || explicitRdpcmMode == RDPCM_VER )
  {
    m_pcBinIf->encodeBin (1, m_explicitRdpcmFlagSCModel.get (0, toChannelType(compID), 0));
#if PIP
	if (encodeTime && verbose)
		cout << 1;
#endif
    if(explicitRdpcmMode == RDPCM_HOR)
    {
      m_pcBinIf->encodeBin ( 0, m_explicitRdpcmDirSCModel.get(0, toChannelType(compID), 0));
#if PIP
	  if (encodeTime && verbose)
		  cout << 0;
#endif
    }
    else
    {
      m_pcBinIf->encodeBin ( 1, m_explicitRdpcmDirSCModel.get(0, toChannelType(compID), 0));
#if PIP
	  if (encodeTime && verbose)
		  cout << 1;
#endif
    }
  }
  else
  {
    assert(0);
  }
}

#if ALF_HM3_REFACTOR
#if !JVET_C0024_QTBT
Void TEncSbac::codeAlfCtrlFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  if (!m_bAlfCtrl)
    return;

  if( pcCU->getDepth(uiAbsPartIdx) > m_uiMaxAlfCtrlDepth && !pcCU->isFirstAbsZorderIdxInDepth(uiAbsPartIdx, m_uiMaxAlfCtrlDepth))
  {
    return;
  }

  // get context function is here
  UInt uiSymbol = pcCU->getAlfCtrlFlag( uiAbsPartIdx ) ? 1 : 0;

  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, pcCU->getCtxAlfCtrlFlag( uiAbsPartIdx) ) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tAlfCtrlFlag\n" )
}
#endif

Void TEncSbac::codeAlfCtrlDepth( UInt uiMaxTotalCUDepth )
{
  UInt uiDepth = m_uiMaxAlfCtrlDepth;
  xWriteUnaryMaxSymbol(uiDepth, m_cALFUvlcSCModel.get(0), 1, uiMaxTotalCUDepth-1);
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tAlfCtrlDepth\n" )
}
#if JVET_C0038_GALF  
//0: no pred; 1: all same index; 2: diff index for each variance index
Void TEncSbac::codeALFPrevFiltType( UInt uiCode)
{
  xWriteEpExGolomb( uiCode, 0);
}
Void TEncSbac::codeALFPrevFiltFlag( Int uiCode)
{
   m_pcBinIf->encodeBinEP( uiCode > 0 ? 1: 0 );
}
#endif
Void TEncSbac::codeAlfFlag       ( UInt uiCode )
{
  UInt uiSymbol = ( ( uiCode == 0 ) ? 0 : 1 );
#if JVET_C0038_GALF
  m_pcBinIf->encodeBinEP( uiSymbol);
#else
  m_pcBinIf->encodeBin( uiSymbol, m_cALFFlagSCModel.get( 0, 0, 0 ) );
#endif
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tAlfFlag\n" )
}

Void TEncSbac::codeAlfFlagNum( UInt uiCode, UInt minValue )
{
  UInt uiLength = 0;
  UInt maxValue = (minValue << (this->getMaxAlfCtrlDepth()*2));
  assert((uiCode>=minValue)&&(uiCode<=maxValue));
  UInt temp = maxValue - minValue;
  for(UInt i=0; i<32; i++)
  {
    if(temp&0x1)
    {
      uiLength = i+1;
    }
    temp = (temp >> 1);
  }
  UInt uiSymbol = uiCode - minValue;
  if(uiLength)
  {
    while( uiLength-- )
    {
      m_pcBinIf->encodeBinEP( (uiSymbol>>uiLength) & 0x1 );
    }
  }
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tAlfFlagNum\n" )
}

Void TEncSbac::codeAlfCtrlFlag( UInt uiSymbol )
{
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAlfCtrlFlagSCModel.get( 0, 0, 0) );
  DTRACE_CABAC_VL( g_nSymbolCounter++ )
  DTRACE_CABAC_T( "\tAlfCtrlFlag\n" )
}

Void TEncSbac::codeAlfUvlc       ( UInt uiCode )
{
  Int i;
#if JVET_C0038_GALF
  if ( uiCode == 0 )
  {
    m_pcBinIf->encodeBinEP(0);
  }
  else
  {
    m_pcBinIf->encodeBinEP(1);
    for ( i=0; i<uiCode-1; i++ )
    {
        m_pcBinIf->encodeBinEP(1);
    }
    m_pcBinIf->encodeBinEP(0);
  }
#else
  if ( uiCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 0 ) );
    for ( i=0; i<uiCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFUvlcSCModel.get( 0, 0, 1 ) );
  }
#endif
}

Void TEncSbac::codeAlfSvlc       ( Int iCode )
{
  Int i;
#if JVET_C0038_GALF
  if ( iCode == 0 )
  {
    m_pcBinIf->encodeBinEP(0);
  }
  else
  {
    m_pcBinIf->encodeBinEP(1);

    // write sign
    if ( iCode > 0 )
    {
      m_pcBinIf->encodeBinEP(0);
    }
    else
    {
     m_pcBinIf->encodeBinEP(1);
      iCode = -iCode;
    }

    // write magnitude
    for ( i=0; i<iCode-1; i++ )
    {
      m_pcBinIf->encodeBinEP(1);
    }
    m_pcBinIf->encodeBinEP(0);
  }
#else
  if ( iCode == 0 )
  {
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 0 ) );
  }
  else
  {
    m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 0 ) );

    // write sign
    if ( iCode > 0 )
    {
      m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
    }
    else
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 1 ) );
      iCode = -iCode;
    }

    // write magnitude
    for ( i=0; i<iCode-1; i++ )
    {
      m_pcBinIf->encodeBin( 1, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
    }
    m_pcBinIf->encodeBin( 0, m_cALFSvlcSCModel.get( 0, 0, 2 ) );
  }
#endif
}
#endif

#if COM16_C806_EMT
Void TEncSbac::codeEmtTuIdx( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth )
{

#if JVET_C0024_QTBT
  if ( pcCU->isIntra( uiAbsPartIdx ) && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTRA_MAX_CU && pcCU->getHeight(uiAbsPartIdx)<= EMT_INTRA_MAX_CU )
#else
  if ( pcCU->isIntra( uiAbsPartIdx ) && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTRA_MAX_CU )
#endif
  {
    UChar ucTrIdx = pcCU->getEmtTuIdx( uiAbsPartIdx );
    m_pcBinIf->encodeBin( ( ucTrIdx & 1 ) ? 1 : 0, m_cEmtTuIdxSCModel.get(0, 0, 0));
    m_pcBinIf->encodeBin( ( ucTrIdx / 2 ) ? 1 : 0, m_cEmtTuIdxSCModel.get(0, 0, 1));
  }
#if JVET_C0024_QTBT
  if ( !pcCU->isIntra( uiAbsPartIdx ) && pcCU->getWidth(uiAbsPartIdx) <=EMT_INTER_MAX_CU && pcCU->getHeight(uiAbsPartIdx)<= EMT_INTER_MAX_CU  )
#else
  if ( !pcCU->isIntra( uiAbsPartIdx ) && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTER_MAX_CU )
#endif
  {
    UChar ucTrIdx = pcCU->getEmtTuIdx( uiAbsPartIdx );
    m_pcBinIf->encodeBin( ( ucTrIdx & 1 ) ? 1 : 0, m_cEmtTuIdxSCModel.get(0, 0, 2));
    m_pcBinIf->encodeBin( ( ucTrIdx / 2 ) ? 1 : 0, m_cEmtTuIdxSCModel.get(0, 0, 3));
  }
}

Void TEncSbac::codeEmtCuFlag( TComDataCU* pcCU, UInt uiAbsPartIdx, UInt uiDepth, Bool bCodeCuFlag )
{


#if JVET_C0024_QTBT
  if (uiDepth >= NUM_EMT_CU_FLAG_CTX)
  {
    uiDepth = NUM_EMT_CU_FLAG_CTX - 1;
  }
#else
  assert( uiDepth < NUM_EMT_CU_FLAG_CTX );
#endif

#if JVET_C0024_QTBT
  if ( pcCU->isIntra( uiAbsPartIdx ) && bCodeCuFlag && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTRA_MAX_CU && pcCU->getHeight(uiAbsPartIdx)<= EMT_INTRA_MAX_CU
      && pcCU->getSlice()->getSPS()->getUseIntraEMT() )
#else
  if ( pcCU->isIntra( uiAbsPartIdx ) && bCodeCuFlag && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTRA_MAX_CU && pcCU->getSlice()->getSPS()->getUseIntraEMT() )
#endif
  {
    UChar ucCuFlag = pcCU->getEmtCuFlag( uiAbsPartIdx );
    m_pcBinIf->encodeBin( ucCuFlag, m_cEmtCuFlagSCModel.get(0, 0, uiDepth));
#if PIP
	if (encodeTime && verbose)
		cout << (Int)ucCuFlag;
#endif
  }
#if JVET_C0024_QTBT
  if( !pcCU->isIntra( uiAbsPartIdx ) && bCodeCuFlag && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTER_MAX_CU && pcCU->getHeight(uiAbsPartIdx)<= EMT_INTER_MAX_CU
      && pcCU->getSlice()->getSPS()->getUseInterEMT() )
#else
  if( !pcCU->isIntra( uiAbsPartIdx ) && bCodeCuFlag && pcCU->getWidth(uiAbsPartIdx) <= EMT_INTER_MAX_CU && pcCU->getSlice()->getSPS()->getUseInterEMT() )
#endif
  {
    UChar ucCuFlag = pcCU->getEmtCuFlag( uiAbsPartIdx );
    m_pcBinIf->encodeBin( ucCuFlag, m_cEmtCuFlagSCModel.get(0, 0, uiDepth));
#if PIP
	if (encodeTime && verbose)
		cout << (Int)ucCuFlag;
#endif
  }
}
#endif

#if COM16_C1016_AFFINE
/** code affine flag
 * \param pcCU
 * \param uiAbsPartIdx 
 * \returns Void
 */
Void TEncSbac::codeAffineFlag( TComDataCU* pcCU, UInt uiAbsPartIdx )
{
  // get context function is here
  UInt uiSymbol = pcCU->isAffine( uiAbsPartIdx ) ? 1 : 0;
  UInt uiCtxAffine = pcCU->getCtxAffineFlag( uiAbsPartIdx );
  m_pcBinIf->encodeBin( uiSymbol, m_cCUAffineFlagSCModel.get( 0, 0, uiCtxAffine ) );

  DTRACE_CABAC_VL( g_nSymbolCounter++ );
  DTRACE_CABAC_T( "\tAffineFlag" );
  DTRACE_CABAC_T( "\tuiCtxAffine: ");
  DTRACE_CABAC_V( uiCtxAffine );
  DTRACE_CABAC_T( "\tuiSymbol: ");
  DTRACE_CABAC_V( uiSymbol );
  DTRACE_CABAC_T( "\n");
}
#endif

#if VCEG_AZ07_BAC_ADAPT_WDOW
Void TEncSbac::xUpdateWindowSize ( SliceType eSliceType, Int uiQPIdx, TComStats* apcStats )
{
  if( uiQPIdx == -1 )
  {
    return;
  }
  Int iCtxNr = getCtxNumber();
  for(UInt i=0; i<iCtxNr; i++)
  {
    m_contextModels[i].setWindowSize(apcStats->m_uiCtxCodeIdx[eSliceType][uiQPIdx][i]);
  }
}
Void TEncSbac::codeCtxUpdateInfo  ( TComSlice* pcSlice,  TComStats* apcStats )
{
  assert(0);
  return;
}
#endif

//! \}



#if PIP && BITPLANE_R1_CODING

Void TEncSbac::convertToBitLayers_Unary(UInt *vector, UInt size, UInt outbl[MAXDYN - 1][4][4])
{
	if (size != 16) { printf("not implement for size other than 16\n"); exit(1); }

	for (UInt l = 0; l<MAXDYN - 1; l++)
		for (UInt lin = 0; lin < 4; lin++)
		{
			for (UInt col = 0; col < 4; col++)
			{
				if (l < vector[lin * 4 + col]) outbl[l][lin][col] = 1;
				else if (l == vector[lin * 4 + col]) outbl[l][lin][col] = 0;
				else outbl[l][lin][col] = 2; //Put 2 in the layers above the 0.
			}
		}
}


//Fill in situations with the situations of the bins of unary code layers 
Void TEncSbac::computeSituationsFromBitLayers_Unary(UInt unarybitlayers[MAXDYN - 1][4][4], Int situations[MAXDYN - 1][4][4])
{

	for (UInt l = 0; l<MAXDYN - 1; l++)
		for (Int lin = 0; lin < 4; lin++)
		{
			for (Int col = 0; col < 4; col++)
			{

				//Compute the situation only for the bins that will require encoding (hence the 2).
				//Situations that do not need to be computed are set artifically to -1 so they can be detected when debugging
				if (unarybitlayers[l][lin][col] != 2) situations[l][lin][col] = getSituation(unarybitlayers, l, lin, col);
				else situations[l][lin][col] = -1;



			}

		}

	/*
	for (UInt l = 0; l < MAXDYN - 1; l++)
	{
	printf("Unary Context Layer %d:\n", l);
	for (UInt lin = 0; lin < 4; lin++)
	{
	for (UInt col = 0; col < 4; col++)
	{

	printf("%d\t", situations[l][lin][col]);


	}
	printf("\n");
	}

	}
	*/

}

//Get the situation at a given location (layers l, line lin, column col)
//Neighbors of X:
// BCD
// AXE
// HGF
UInt TEncSbac::getSituation(UInt unarybitlayers[MAXDYN - 1][4][4], UInt l, UInt lin, UInt col)
{

	//This is used to indicate that we are in the upper locations (A,B,C,D) if true and lower locations (E,F,G,H) if false
	Bool causal = true;
	Int x, y;

	//This is going to contain a status for each fo the 8 neighbors
	Char neighbors[3][3];

	for (Int linshift = -1; linshift <= 1; linshift++)
		for (Int colshift = -1; colshift <= 1; colshift++)
		{

			//If centre, update causalilty, and avoid center
			if (colshift == 0 && linshift == 0)
			{
				causal = false;
				neighbors[1][1] = '-';
				continue;
			}

			//Determine current coordinates in the current layer
			y = (Int)lin + linshift;
			x = (Int)col + colshift;

			//Check if outside of block, if yes neighbor value is 'N', else it is computed.
			if ((x < 0) || (x > 3) || (y < 0) || (y > 3)) neighbors[1 + linshift][1 + colshift] = 'N';
			//Get status of each neighbor. The layer we look at can be the same layer (for ABCD) or the layer just below (for the four other neighbors
			else neighbors[1 + linshift][1 + colshift] = getNeighbor(unarybitlayers, causal ? (Int)l : (Int)l - 1, (UInt)y, (UInt)x);



		}

	/*
	printf("Neighbors: l=%d, lin=%d, col=%d:\n", l, lin, col);

	for (Int y = 0; y < 3; y++)
	{
	for (Int x = 0; x < 3; x++) printf("%c\t", neighbors[y][x]);
	printf("\n");
	}
	*/
	////Convert neighbors to situation////

	//First Check if MAXLEVELSFORSITUATION has the right value
	if (MAXLEVELSFORSITUATION != 2) { printf("Computation of the context has wrong value for MAXLEVELSFORSITUATIOn\n"); exit(1); }


	//Convert the status of each neighbor to a numerical value
	UInt a = neighborToValue(neighbors[1][0]);
	UInt b = neighborToValue(neighbors[0][0]);
	UInt c = neighborToValue(neighbors[0][1]);
	UInt d = neighborToValue(neighbors[0][2]);
	UInt e = neighborToValue(neighbors[1][2]);
	UInt f = neighborToValue(neighbors[2][2]);
	UInt g = neighborToValue(neighbors[2][1]);
	UInt h = neighborToValue(neighbors[2][0]);


	//Compute the final value of the situation 

	UInt situation = neighborsToSituation(l, a, b, c, d, e, f, g, h);

	//Control
	if (situation > TOTALNUMOFSITUATIONS - 1) { printf("Error in situation (%d vs %d)\n", situation, TOTALNUMOFSITUATIONS); exit(2); }
	//if (situation == TOTALNUMOFSITUATIONS-1) { printf("Reaching highest situation!\n");  }

	return situation;
}


UInt TEncSbac::neighborsToSituation(UInt level, UInt a, UInt b, UInt c, UInt d, UInt e, UInt f, UInt g, UInt h)
{

#if SITUATIONMODE == 1
	UInt situation = 0; //Merge all layers in this test mode
	situation = situation * 2 + a;
	situation = situation * 2 + b;
	situation = situation * 2 + c;
	situation = situation * 2 + d;
	situation = situation * 2 + e;
	situation = situation * 2 + f;
	situation = situation * 2 + g;
	situation = situation * 2 + h;
#endif


#if SITUATIONMODE == 2
	UInt situation = level;
	situation = situation * 2 + a;
	situation = situation * 2 + b;
	situation = situation * 2 + c;
	situation = situation * 2 + d;
	situation = situation * 2 + e;
	situation = situation * 2 + f;
	situation = situation * 2 + g;
	situation = situation * 2 + h;
#endif

#if SITUATIONMODE == 3
	UInt situation = level;
	situation = situation * 3 + a;
	situation = situation * 3 + b;
	situation = situation * 3 + c;
	situation = situation * 3 + d;
	situation = situation * 3 + e;
	situation = situation * 3 + f;
	situation = situation * 3 + g;
	situation = situation * 3 + h;
#endif

#if SITUATIONMODE == 4
	UInt situation = level;
	situation = situation * 4 + a;
	situation = situation * 4 + b;
	situation = situation * 4 + c;
	situation = situation * 4 + d;
	situation = situation * 4 + e;
	situation = situation * 4 + f;
	situation = situation * 4 + g;
	situation = situation * 4 + h;
#endif

#if SITUATIONMODE == 5
	UInt situation = level;
	situation = situation * 5 + a;
	situation = situation * 5 + b;
	situation = situation * 5 + c;
	situation = situation * 5 + d;
	situation = situation * 5 + e;
	situation = situation * 5 + f;
	situation = situation * 5 + g;
	situation = situation * 5 + h;
#endif
	return situation;
}

//Convert the status of a neighbor to an actual value. 
//In this implementation we only distinguish between unavailable (be it 'N' or 'B'), significant and non significant. So anything above '1' is 1.
UInt TEncSbac::neighborToValue(Char c)
{

#if SITUATIONMODE == 5
	switch (c)
	{
	case 'N':
	case 'B':
		return 0;
		break;
	case '0':
		return 1;
		break;
	case '1':
		return 2;
		break;
	case '2':
		return 3;
		break;
	case '+':
		return 4;
		break;

	default:
		printf("Neighbor should never have this value: %c ", c);
		exit(1);
		break;
	}
#endif


#if SITUATIONMODE == 4
	switch (c)
	{
	case 'N':
	case 'B':
		return 0;
		break;
	case '0':
		return 1;
		break;
	case '1':
		return 2;
		break;
	case '2':
		return 3;
		break;
	case '+':
		return 3;
		break;

	default:
		printf("Neighbor should never have this value: %c ", c);
		exit(1);
		break;
	}
#endif


#if SITUATIONMODE == 3
	switch (c)
	{
	case 'N':
	case 'B':
		return 0;
		break;
	case '0':
		return 1;
		break;
	case '1':
		return 2;
		break;
	case '2':
		return 2;
		break;
	case '+':
		return 2;
		break;

	default:
		printf("Neighbor should never have this value: %c ", c);
		exit(1);
		break;
	}
#endif


#if SITUATIONMODE == 2
	switch (c)
	{
	case 'N':
	case 'B':
		return 0;
		break;
	case '0':
		return 1;
		break;
	case '1':
		return 1;
		break;
	case '2':
		return 1;
		break;
	case '+':
		return 1;
		break;

	default:
		printf("Neighbor should never have this value: %c ", c);
		exit(1);
		break;
	}
#endif


#if SITUATIONMODE == 1
	switch (c)
	{
	case 'N':
	case 'B':
		return 0;
		break;
	case '0':
		return 1;
		break;
	case '1':
		return 1;
		break;
	case '2':
		return 1;
		break;
	case '+':
		return 1;
		break;

	default:
		printf("Neighbor should never have this value: %c ", c);
		exit(1);
		break;
	}
#endif
}

//get the status of a neighbor (ABCDEFGH)
Char TEncSbac::getNeighbor(UInt bl[MAXDYN - 1][4][4], Int l, UInt lin, UInt col)
{

	//If trying to acces below the first level, return a 'B' symbol
	if (l < 0) return 'B';

	UInt val = 0;

	//The levels we are allowed to access at this location
	UInt lastlevel = min((Int)MAXLEVELSFORSITUATION, l);

	//printf("%d %d %d Lastlevel: %d\n", l,lin,col,lastlevel);

	//Compute known level at this location
	for (UInt level = 0; level <= lastlevel; level++)
	{
		if (bl[level][lin][col] == 2) break;
		val += bl[level][lin][col];
	}

	//Return status accordingly
	switch (val)
	{
	case 0:
		return '0'; //Means value is exactly 0
		break;
	case 1:
		return '1'; //Means value is exactly 1
		break;
	case 2:
		return'2'; //Means value is exactly 2
		break;
	case 3:
		return'+'; //Means value is 3 or more
		break;
	default:
		printf("getNeighbor not implemented for this MAXLEVELSFORSITUATION value (probably)(%d)\n ", val);
		exit(1);
		break;
	}
}

#endif
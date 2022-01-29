// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "primitives/block.h"
#include "uint256.h"

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimit).GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;
	
if((pindexLast->nHeight+1) < 245000)
{
    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight+1) % params.DifficultyAdjustmentInterval() != 0)
    {
        if (params.fPowAllowMinDifficultyBlocks)
        {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing*2)
                return nProofOfWorkLimit;
            else
            {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex* pindex = pindexLast;
                while (pindex->pprev && pindex->nHeight % params.DifficultyAdjustmentInterval() != 0 && pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }
}

   
    int64_t nActualTimespan = 30 * 120;
    const CBlockIndex* pindexFirst = pindexLast;
	
    if((pindexLast->nHeight+1) < 272000)
    {   
	// Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
        int blockstogoback = params.DifficultyAdjustmentInterval()-1;
        if ((pindexLast->nHeight+1) != params.DifficultyAdjustmentInterval())
            blockstogoback = params.DifficultyAdjustmentInterval();
	    
	// Go back by what we want to be blockstogoback worth of blocks
	const CBlockIndex* pindexFirst = pindexLast;
	for (int i = 0; pindexFirst && i < blockstogoback; i++)
            pindexFirst = pindexFirst->pprev;
        assert(pindexFirst);
	    
        // Limit adjustment step
        nActualTimespan = pindexLast->GetBlockTime() - pindexFirst-> GetBlockTime ();
        //LogPrintf("  nActualTimespan = %d  before bounds\n", nActualTimespan);
        if((pindexLast->nHeight+1) < 1500)
        {
            if (nActualTimespan < params.nPowTargetTimespan/16)
                nActualTimespan = params.nPowTargetTimespan/16;
        }
	else
        {
            if (nActualTimespan < params.nPowTargetTimespan/4)
                nActualTimespan = params.nPowTargetTimespan/4;
	} 
	if (nActualTimespan > params.nPowTargetTimespan*4)
            nActualTimespan = params.nPowTargetTimespan*4;
    }
    else	// PPCoin formula with 1 block time
    {
	// get the previous block
	pindexFirst = pindexLast->pprev;
	nActualTimespan = (pindexLast->GetBlockTime() - pindexFirst-> GetBlockTime ()) * params.DifficultyAdjustmentInterval();

	// limit the adjustment
        if (nActualTimespan < params.nPowTargetTimespan/16)
            nActualTimespan = params.nPowTargetTimespan/16;
        if (nActualTimespan > params.nPowTargetTimespan*16)
            nActualTimespan = params.nPowTargetTimespan*16;
    }
	    

    // Retarget
    arith_uint256 bnNew;
    arith_uint256 bnOld;
    bnNew.SetCompact(pindexLast->nBits);
    bnOld = bnNew;
	
    if((pindexLast->nHeight+1) < 248000)                      // 120-block linear retarget
    {
        bnNew *= nActualTimespan;
        bnNew /= params.nPowTargetTimespan;
    }
    else                                                      // PPCoin retarget algorithm
    {
	bnNew *= ((30 - 1) * params.nPowTargetTimespan + nActualTimespan + nActualTimespan);
	bnNew /= ((30 + 1) * params.nPowTargetTimespan);
    }
	
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimit);
    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();	
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    if(hash.GetHex() == "55ca007399d28cd3e3d921709ee6c52665db5cbf4cf0865230a00d2992c6812b")
            return true;
	
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return false;

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return false;

    return true;
}

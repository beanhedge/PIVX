// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "accumulators.h"
#include "chain.h"
#include "primitives/zerocoin.h"
#include "main.h"
#include "stakeinput.h"
#include "wallet.h"

// The zPIV block index is the first appearance of the accumulator checksum that was used in the spend
// note that this also means when staking that this checksum should be from a block that is beyond 60 minutes old and
// 100 blocks deep.
CBlockIndex* CZPivStake::GetIndexFrom()
{
    if (pindexFrom)
        return pindexFrom;

    uint32_t nChecksum = spend->getAccumulatorChecksum();
    int nHeightChecksum = GetChecksumHeight(nChecksum, spend->getDenomination());
    if (nHeightChecksum < Params().Zerocoin_StartHeight()) {
        pindexFrom = nullptr;
    } else {
        //note that this will be a nullptr if the height DNE
        pindexFrom = chainActive[nHeightChecksum];
    }

    return pindexFrom;
}

CAmount CZPivStake::GetValue()
{
    return spend->getDenomination() * COIN;
}

//Use the first accumulator checkpoint that occurs 60 minutes after the block being staked from
bool CZPivStake::GetModifier(uint64_t& nStakeModifier)
{
    CBlockIndex* pindex = GetIndexFrom();
    if (!pindex)
        return false;

    int64_t nTimeBlockFrom = pindex->GetBlockTime();
    while (true) {
        if (pindex->GetBlockTime() - nTimeBlockFrom > 60*60) {
            nStakeModifier = pindex->nAccumulatorCheckpoint.Get64();
            return true;
        }

        if (pindex->nHeight + 1 <= chainActive.Height())
            pindex = chainActive.Next(pindex);
        else
            return false;
    }
}

CDataStream CZPivStake::GetUniqueness()
{
    //The unique identifier for a ZPIV is the serial
    CDataStream ss(SER_GETHASH, 0);
    ss << spend->getCoinSerialNumber();
    return ss;
}

bool CZPivStake::CreateTxIn(CTxIn& txIn)
{
    return true;
}

bool CZPivStake::GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey)
{
    return true;
}

bool CZPivStake::GetTxFrom(CTransaction& tx)
{
    return false;
}


//!PIV Stake
bool CPivStake::SetInput(std::pair<CTransaction*, unsigned int> input)
{
    this->pcoin = input;
    return true;
}

bool CPivStake::GetTxFrom(CTransaction& tx)
{
    tx = *pcoin.first;
    return true;
}

bool CPivStake::CreateTxIn(CTxIn& txIn)
{
    txIn = CTxIn(pcoin.first->GetHash(), pcoin.second);
    return true;
}

CAmount CPivStake::GetValue()
{
    return pcoin.first->vout[pcoin.second].nValue;
}

bool CPivStake::GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey)
{
    vector<valtype> vSolutions;
    txnouttype whichType;
    CScript scriptPubKeyOut;
    CScript scriptPubKeyKernel = pcoin.first->vout[pcoin.second].scriptPubKey;
    if (!Solver(scriptPubKeyKernel, whichType, vSolutions)) {
        LogPrintf("CreateCoinStake : failed to parse kernel\n");
        return false;
    }

    if (whichType != TX_PUBKEY && whichType != TX_PUBKEYHASH)
        return false; // only support pay to public key and pay to address

    if (whichType == TX_PUBKEYHASH) // pay to address type
    {
        //convert to pay to public key type
        CKey key;
        if (!keystore.GetKey(uint160(vSolutions[0]), key))
            return false;

        scriptPubKeyOut << key.GetPubKey() << OP_CHECKSIG;
    } else
        scriptPubKeyOut = scriptPubKeyKernel;

    return true;
}

bool CPivStake::GetModifier(uint64_t& nStakeModifier)
{
    int nStakeModifierHeight = 0;
    int64_t nStakeModifierTime = 0;
    GetIndexFrom();
    if (!pindexFrom)
        return false;

    if (!GetKernelStakeModifier(pindexFrom->GetBlockHash(), nStakeModifier, nStakeModifierHeight, nStakeModifierTime, false)) {
        LogPrintf("CheckStakeKernelHash(): failed to get kernel stake modifier \n");
        return false;
    }

    return true;
}

CDataStream CPivStake::GetUniqueness()
{
    //The unique identifier for a PIV stake is the outpoint
    CDataStream ss(SER_NETWORK, 0);
    ss << pcoin.second << pcoin.first->GetHash();
    return ss;
}

//The block that the UTXO was added to the chain
CBlockIndex* CPivStake::GetIndexFrom()
{
    uint256 hashBlock = 0;
    CTransaction tx;
    if (GetTransaction(pcoin.first->GetHash(), tx, hashBlock, true)) {
        // If the index is in the chain, then set it as the "index from"
        if (mapBlockIndex.count(hashBlock)) {
            CBlockIndex* pindex = mapBlockIndex.at(hashBlock);
            if (chainActive.Contains(pindex))
                pindexFrom = pindex;
        }
    }

    return pindexFrom;
}
// Copyright (c) 2017 The PIVX developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef PIVX_STAKEINPUT_H
#define PIVX_STAKEINPUT_H

class CKeyStore;
class CWallet;
class CWalletTx;

class CStakeInput
{
protected:
    CBlockIndex* pindexFrom;

public:
    virtual ~CStakeInput(){};
    virtual CBlockIndex* GetIndexFrom() = 0;
    virtual bool CreateTxIn(CWallet* pwallet, CTxIn& txIn) = 0;
    virtual bool GetTxFrom(CTransaction& tx) = 0;
    virtual CAmount GetValue() = 0;
    virtual bool GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey) = 0;
    virtual bool GetModifier(uint64_t& nStakeModifier) = 0;
    virtual CDataStream GetUniqueness() = 0;
};

class CZPivStake : public CStakeInput
{
private:
    libzerocoin::CoinSpend* spend;
    CZerocoinMint mint;

public:
    explicit CZPivStake(CZerocoinMint mint)
    {
        this->mint = mint;
    }

    explicit CZPivStake(libzerocoin::CoinSpend spend)
    {
        this->spend = &spend;
    }

    CBlockIndex* GetIndexFrom() override;
    bool GetTxFrom(CTransaction& tx) override;
    CAmount GetValue() override;
    bool GetModifier(uint64_t& nStakeModifier) override;
    CDataStream GetUniqueness() override;
    bool CreateTxIn(CWallet* pwallet, CTxIn& txIn) override;
    bool GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey) override;
};

class CPivStake : public CStakeInput
{
private:
    CTransaction txFrom;
    unsigned int nPosition;
public:
    CPivStake()
    {

    }

    bool SetInput(CTransaction txPrev, unsigned int n);

    CBlockIndex* GetIndexFrom() override;
    bool GetTxFrom(CTransaction& tx) override;
    CAmount GetValue() override;
    bool GetModifier(uint64_t& nStakeModifier) override;
    CDataStream GetUniqueness() override;
    bool CreateTxIn(CWallet* pwallet, CTxIn& txIn) override;
    bool GetScriptPubKeyTo(const CKeyStore& keystore, CScript& scriptPubKey) override;
};


#endif //PIVX_STAKEINPUT_H

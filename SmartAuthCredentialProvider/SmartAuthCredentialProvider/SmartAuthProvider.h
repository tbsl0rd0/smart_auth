//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#include <credentialprovider.h>
#include <windows.h>
#include <strsafe.h>

#include "CommandWindow.h"
#include "SmartAuthCredential.h"
#include "MessageCredential.h"
#include "helpers.h"
#include "RegCheck.h"

#define MAX_CREDENTIALS 3
#define MAX_DWORD   0xffffffff        // maximum DWORD

class CCommandWindow;
class SmartAuthCredential;
class CMessageCredential;

class SmartAuthProvider : public ICredentialProvider
{
  public:
    // IUnknown
    STDMETHOD_(ULONG, AddRef)()
    {
        return _cRef++;
    }
    
    STDMETHOD_(ULONG, Release)()
    {
        LONG cRef = _cRef--;
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    STDMETHOD (QueryInterface)(REFIID riid, void** ppv)
    {
        HRESULT hr;
        if (IID_IUnknown == riid || 
            IID_ICredentialProvider == riid)
        {
            *ppv = this;
            reinterpret_cast<IUnknown*>(*ppv)->AddRef();
            hr = S_OK;
        }
        else
        {
            *ppv = NULL;
            hr = E_NOINTERFACE;
        }
        return hr;
    }

  public:
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
                                      __out DWORD* pdwDefault,
                                      __out BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, 
                                   __out ICredentialProviderCredential** ppcpc);

    friend HRESULT SmartAuthProvider_CreateInstance(REFIID riid, __deref_out void** ppv);

  public:
	void OnConnectStatusChanged();

  protected:
    SmartAuthProvider();
    __override ~SmartAuthProvider();
    
  private:
    
    HRESULT _EnumerateOneCredential(__in DWORD dwCredientialIndex,
                                    __in PCWSTR pwzUsername
                                    );
    HRESULT _EnumerateSetSerialization();

    // Create/free enumerated credentials.
    HRESULT _EnumerateCredentials();
    void _ReleaseEnumeratedCredentials();
    void _CleanupSetSerialization();


private:
	
    LONG              _cRef;
    SmartAuthCredential *_rgpCredentials[MAX_CREDENTIALS];  // Pointers to the credentials which will be enumerated by 
	
    DWORD                                   _dwNumCreds;
    KERB_INTERACTIVE_UNLOCK_LOGON*          _pkiulSetSerialization;
    DWORD                                   _dwSetSerializationCred; //index into rgpCredentials for the SetSerializationCred
    bool                                    _bAutoSubmitSetSerializationCred;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO      _cpus;

	RegFlag rf;

	// 하드웨어 부분
	CCommandWindow              *_pCommandWindow;       // Emulates external events.
	SmartAuthCredential           *_pCredential;          // Our "connected" credential.
	CMessageCredential          *_pMessageCredential;   // Our "disconnected" credential.
	ICredentialProviderEvents   *_pcpe;                    // Used to tell our owner to re-enumerate credentials.
	UINT_PTR                    _upAdviseContext;       // Used to tell our owner who we are when asking to 
														// re-enumerate credentials.
};

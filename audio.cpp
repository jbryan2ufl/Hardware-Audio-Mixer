#include <iostream>
#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <thread>
#include <chrono>
#include <psapi.h>
#include <windows.h>

__CRT_UUID_DECL(IAudioMeterInformation, 0xC02216F6, 0x8C67, 0x4B5B, 0x9D, 0x00, 0xD0, 0x08, 0xE7, 0x3E, 0x00, 0x64);

MIDL_INTERFACE("C02216F6-8C67-4B5B-9D00-D008E73E0064")
IAudioMeterInformation : public IUnknown
{
public:
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetPeakValue( 
        /* [out] */ float *pfPeak) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetMeteringChannelCount( 
        /* [out] */ 
          UINT *pnChannelCount) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE GetChannelsPeakValues( 
        /* [in] */ UINT32 u32ChannelCount,
        /* [size_is][out] */ float *afPeakValues) = 0;
    
    virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE QueryHardwareSupport( 
        /* [out] */ 
          DWORD *pdwHardwareSupportMask) = 0;
};


int main() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = NULL;
    IMMDeviceCollection* pCollection = NULL;
    IMMDevice* pEndpoint = NULL;
    IAudioSessionManager2* pSessionManager = NULL;
    IAudioSessionEnumerator* pSessionEnumerator = NULL;
    IAudioSessionControl* pSessionControl = NULL;
    IAudioSessionControl2* pSessionControl2 = NULL;

    // Initialize COM
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        std::cerr << "Error: CoInitialize failed" << std::endl;
        return 1;
    }

    // Create the device enumerator
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Error: CoCreateInstance failed" << std::endl;
        CoUninitialize();
        return 1;
    }

	while (true)
	{
		// Enumerate audio endpoint devices
		hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
		if (FAILED(hr)) {
			std::cerr << "Error: EnumAudioEndpoints failed" << std::endl;
			pEnumerator->Release();
			CoUninitialize();
			return 1;
		}

		UINT deviceCount;
		pCollection->GetCount(&deviceCount);

		for (UINT i = 0; i < deviceCount; i++) {
			// Get the next audio endpoint
			hr = pCollection->Item(i, &pEndpoint);
			if (FAILED(hr)) {
				std::cerr << "Error: Item failed" << std::endl;
				break;
			}

			// Get the session manager
			hr = pEndpoint->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
			if (FAILED(hr)) {
				std::cerr << "Error: Activate failed" << std::endl;
				pEndpoint->Release();
				continue;
			}

			// Get the session enumerator
			hr = pSessionManager->GetSessionEnumerator(&pSessionEnumerator);
			if (FAILED(hr)) {
				std::cerr << "Error: GetSessionEnumerator failed" << std::endl;
				pSessionManager->Release();
				pEndpoint->Release();
				continue;
			}

			int sessionCount;
			pSessionEnumerator->GetCount(&sessionCount);
			for (int j = 0; j < sessionCount; j++) {
				// Get the session control
				hr = pSessionEnumerator->GetSession(j, &pSessionControl);
				hr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
				if (FAILED(hr)) {
					std::cerr << "Error: GetSession failed" << std::endl;
					continue;
				}

				ISimpleAudioVolume* pSimpleVolume = NULL;
				hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pSimpleVolume);
				if (SUCCEEDED(hr)) {
					IAudioMeterInformation* pAudioMeterInfo = NULL;
					hr = pSessionControl->QueryInterface(__uuidof(IAudioMeterInformation), (void**)&pAudioMeterInfo);
					if (SUCCEEDED(hr)) {
						float peakValue;
						hr = pAudioMeterInfo->GetPeakValue(&peakValue);
						if (SUCCEEDED(hr)) {
							float newVolume = 0.5f;
							float defaultVolume = 1.0f;
							if (peakValue != 0)
							{
								// LPWSTR string{};
								// pSessionControl->GetDisplayName(&string);
								// std::wcout << string << ' ';
								// std::wcout << "ICON? " << string << ' ';
								// std::cout << "Peak value: " << peakValue << ' ';
								// std::cout << "THIS SHIT GOT VOLUME!!!\n";
								// hr = pSimpleVolume->SetMasterVolume(newVolume, NULL);

								DWORD id{};
								pSessionControl2->GetProcessId(&id);

								TCHAR szExePath[MAX_PATH];
								HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);
								if (hProcess != NULL) {
									DWORD dwSize = GetModuleFileNameEx(hProcess, NULL, szExePath, MAX_PATH);
									if (dwSize > 0) {
										// std::wcout << L"Executable Name: " << szExePath << std::endl;
										HICON hIcon = ExtractIcon(nullptr, szExePath, 0);
										ICONINFO iconInfo;
										if (!GetIconInfo(hIcon, &iconInfo))
										{
											std::cerr << "Getting icon info failed." << std::endl;
										}


										BITMAP bmp;
										if (!GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp))
										{
											std::cerr << "Getting BMP failed." << std::endl;
										}
										// LONG width = bmp.bmWidth;
										// LONG height = bmp.bmHeight;
										// std::wcout << "THE ICON FOR " << szExePath << " IS " << width << 'x' << height << '\n';
									} else {
										std::cerr << "Error: GetModuleFileNameEx failed" << std::endl;
									}
									CloseHandle(hProcess);
								} else {
									std::cerr << "Error: OpenProcess failed" << std::endl;
								}
							}
							else
							{
								// hr = pSimpleVolume->SetMasterVolume(defaultVolume, NULL);
							}
						} else {
							std::cerr << "Error: GetPeakValue failed" << std::endl;
						}

						

						pAudioMeterInfo->Release();
					} else {
						std::cerr << "Error: QueryInterface for IAudioMeterInformation failed" << std::endl;
					}

					pSimpleVolume->Release();
				} else {
					std::cerr << "Error: QueryInterface for ISimpleAudioVolume failed" << std::endl;
				}

				pSessionControl->Release();
				pSessionControl2->Release();
			}

			pSessionEnumerator->Release();
			pSessionManager->Release();
			pEndpoint->Release();
		}

		pCollection->Release();
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    pEnumerator->Release();
    CoUninitialize();
    return 0;
}
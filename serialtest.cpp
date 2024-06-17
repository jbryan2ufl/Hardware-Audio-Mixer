#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>
#include <iostream>
#include <combaseapi.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>
#include <endpointvolume.h>
#include <thread>
#include <chrono>
#include <psapi.h>
#include <windows.h>
#include <vector>

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

int read_integer(boost::asio::serial_port &serial) {
  boost::asio::streambuf buf;
  boost::asio::read_until(serial, buf, "\n");
  std::istream is(&buf);
  std::string line;
  std::getline(is, line);
  try {
    int number = std::stoi(line);  // Convert string to integer
    return number;
  } catch (std::invalid_argument& e) {
    std::cerr << "Invalid number received: " << line << std::endl;
    return 0;  // or handle the error as needed
  } catch (std::out_of_range& e) {
    std::cerr << "Number out of range: " << line << std::endl;
    return 0;  // or handle the error as needed
  }
}

void send_message(boost::asio::serial_port &serial, const std::string &message) {
  boost::asio::write(serial, boost::asio::buffer(message));
}

void send_bytes(boost::asio::serial_port &serial, const std::vector<uint8_t> &data) {
  boost::asio::write(serial, boost::asio::buffer(data));
}

std::string read_message(boost::asio::serial_port &serial) {
  boost::asio::streambuf buf;
  boost::asio::read_until(serial, buf, "\n");
  std::istream is(&buf);
  std::string line;
  std::getline(is, line);
  std::cout << "Received: " << line << std::endl;
  return line;
}

int main() {
	boost::asio::io_context io;
    boost::asio::serial_port serial(io, "COM4");
    serial.set_option(boost::asio::serial_port_base::baud_rate(9600));

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
						if (peakValue != 0)
						{
							DWORD id{};
							pSessionControl2->GetProcessId(&id);
							TCHAR szExePath[MAX_PATH];
							HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, id);
							std::cout << "AUDIO SOURCE FOUND! " << szExePath << '\n';

							while (true) {
								float f{read_integer(serial)/100.0f};
								pSimpleVolume->SetMasterVolume(f, NULL);
							}


							
							// std::wcout << szExePath << ' ';
							// std::cout << "Peak value: " << peakValue << ' ';
							// std::cout << "THIS SHIT GOT VOLUME!!!\n";
							// hr = pSimpleVolume->SetMasterVolume(newVolume, NULL);


							// if (hProcess != NULL) {
							// 	DWORD dwSize = GetModuleFileNameEx(hProcess, NULL, szExePath, MAX_PATH);
							// 	if (dwSize > 0) {
							// 		HICON hIcon = ExtractIcon(nullptr, szExePath, 0);
							// 		if (hIcon == nullptr) {
							// 			std::cerr << "Error: ExtractIcon failed." << std::endl;
							// 			continue;
							// 		}

							// 		ICONINFO iconInfo;
							// 		if (!GetIconInfo(hIcon, &iconInfo)) {
							// 			std::cerr << "Error: GetIconInfo failed." << std::endl;
							// 			DestroyIcon(hIcon);
							// 			continue;
							// 		}

							// 		// Create a copy of the color bitmap with a DIB section
							// 		HBITMAP hBitmap = (HBITMAP)CopyImage(iconInfo.hbmColor, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
							// 		if (hBitmap == nullptr) {
							// 			std::cerr << "Error: CopyImage failed." << std::endl;
							// 			DestroyIcon(hIcon);
							// 			continue;
							// 		}

							// 		// Retrieve the bitmap information
							// 		BITMAP bmp;
							// 		if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) {
							// 			std::cerr << "Error: GetObject failed." << std::endl;
							// 			DeleteObject(hBitmap);
							// 			DestroyIcon(hIcon);
							// 			continue;
							// 		}

							// 		if (bmp.bmWidth != 32 || bmp.bmHeight != 32) {
							// 			std::cerr << "Error: Bitmap dimensions are not 32x32." << std::endl;
							// 			return false;
							// 		}

							// 		HDC hdc = CreateCompatibleDC(nullptr);
							// 		if (hdc == nullptr) {
							// 			std::cerr << "Error: CreateCompatibleDC failed." << std::endl;
							// 			continue;
							// 		}

							// 		// Select the bitmap into the compatible DC
							// 		HGDIOBJ hOldBitmap = SelectObject(hdc, hBitmap);
							// 		if (hOldBitmap == nullptr) {
							// 			std::cerr << "Error: SelectObject failed." << std::endl;
							// 			DeleteDC(hdc);
							// 			continue;
							// 		}

							// 		BYTE pixelArray[32 * 32 * 4];

							// 		BITMAPINFO bmi;
							// 		ZeroMemory(&bmi, sizeof(BITMAPINFO));
							// 		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
							// 		bmi.bmiHeader.biWidth = 32; // Width of the bitmap
							// 		bmi.bmiHeader.biHeight = -32; // Height of the bitmap (negative to ensure top-down orientation)
							// 		bmi.bmiHeader.biPlanes = 1;
							// 		bmi.bmiHeader.biBitCount = 32; // Bits per pixel (ARGB format)
							// 		bmi.bmiHeader.biCompression = BI_RGB;

							// 		// Retrieve the pixel data from the bitmap
							// 		if (!GetDIBits(hdc, hBitmap, 0, 32, pixelArray, &bmi, DIB_RGB_COLORS)) {
							// 			std::cerr << "Error: GetDIBits failed." << std::endl;
							// 			SelectObject(hdc, hOldBitmap);
							// 			DeleteDC(hdc);
							// 			continue;
							// 		}

							// 		int size = 32 * 32 * 4;
							// 		int threshold = 128;

							// 		for (int i = 3, count = 0; i < size; i += 4, ++count) { // Start from the alpha channel byte
							// 			std::cout << (pixelArray[i+2] > threshold ? "1" : "0");
							// 			if (count % 32 == 31) { // Add a newline after every 32 characters
							// 				std::cout << std::endl;
							// 			}
							// 		}

							// 		// Cleanup resources
							// 		DeleteObject(hBitmap);
							// 		DestroyIcon(hIcon);
							// 	} else {
							// 		std::cerr << "Error: GetModuleFileNameEx failed" << std::endl;
							// 	}
							// 	CloseHandle(hProcess);
							// } else {
							// 	std::cerr << "Error: OpenProcess failed" << std::endl;
							// }
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
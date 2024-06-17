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
#include <boost/asio.hpp>
#include <boost/asio/serial_port.hpp>

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

void send_message(boost::asio::serial_port &serial, const std::string &message) {
  boost::asio::write(serial, boost::asio::buffer(message));
}

void send_bytes(boost::asio::serial_port &serial, const std::vector<uint8_t> &data) {
  boost::asio::write(serial, boost::asio::buffer(data));
}

void read_message(boost::asio::serial_port &serial) {
  boost::asio::streambuf buf;
  boost::asio::read_until(serial, buf, "\n");
  std::istream is(&buf);
  std::string line;
  std::getline(is, line);
  std::cout << "Received: " << line << std::endl;
}

int main() {
  try {
    boost::asio::io_context io;
    boost::asio::serial_port serial(io, "COM4");
    serial.set_option(boost::asio::serial_port_base::baud_rate(9600));


  } catch (boost::system::system_error &e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}

while (true) {
      read_message(serial);  // Read and print the message
}
 
// c++ -std=c++11 -o microphoneFMTX2 microphoneFMTX2.cpp -lSoapySDR -lrtaudio -lliquid -lpthread -Wall
// microphoneFMTX2 "driver=hackrf"

#include <iostream>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>
#include <cstdlib>
#include <cstddef>
#include <chrono>
#include <string>
#include <cstdint>
#include <complex>
#include <csignal>

#include <liquid/liquid.h>

#include <rtaudio/RtAudio.h>

#include <unistd.h>

struct Info{
	volatile int loop;
	msresamp_rrrf iqSampler;
	class AMmod *am;
	msresamp_crcf iqSampler2;
	SoapySDR::Device *device;
	SoapySDR::Stream *txStream;
	freqmod mod;
};

struct Info info;

using namespace std;

#define	BLOCK_SIZE2 200000

#define	BLOCK_SIZE 2048

float buf [BLOCK_SIZE2],r[2*BLOCK_SIZE2];
float buf2 [BLOCK_SIZE2],r2[2*BLOCK_SIZE2];

int SendData(struct Info *data,unsigned int frames,short *buf);

int printinfo();

int input( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus /*status*/, void *datain );

void sigIntHandler(const int);


int main(int argc, char** argv)
{
	
    const double frequency = 85.6e6;  //center frequency to 500 MHz
    const double sample_rate = 400000;    //sample rate to 5 MHz
    float As = 60.0f;
    	
    std::string argStr(argv[1]);
    
    std::vector<size_t> channels;

	channels = {0};
	
  	info.loop = 1;
  	
  	printinfo();
  	
    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        std::cerr << "No device!" << std::endl;
        return EXIT_FAILURE;
    }

	info.device=device;

    device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);

    cout << "Sample rate: " << sample_rate/1e6 << " MHz" << endl;
    
     device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     device->setGain(SOAPY_SDR_TX, 0, 32.0);
     
   //  device->setAntenna(SOAPY_SDR_TX, 0, BAND1);
     
     
    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    info.txStream=txStream;

    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
	freqmod mod = freqmod_create(0.5); 
	
	info.mod=mod;
	
	
	float Ratio1 = (float)(12500/ (float)48000);
	
	float Ratio2 = (float)(sample_rate/(float)12500);
	
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	info.iqSampler=iqSampler;
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	info.iqSampler2=iqSampler2;
	
	
	std::vector<void *> buffs(2);
	

  RtAudio adc;
  if ( adc.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    exit( 1 );
  }
  
  
  RtAudio::StreamParameters Params;
  Params.deviceId = adc.getDefaultInputDevice();
  Params.deviceId = 2;
  Params.nChannels = 1;
  Params.firstChannel = 0;
  
  
  signal(SIGINT, sigIntHandler);

  unsigned int bufferFrames = 2040;
  
  int fs=48000;
	
  try {
    adc.openStream( NULL, &Params, RTAUDIO_SINT16, fs, &bufferFrames, &input, (void *)&info );
    adc.startStream();
  }
  catch ( RtAudioError& e ) {
    std::cout << '\n' << e.getMessage() << '\n' << std::endl;
    goto cleanup;
  }
	
  std::cout << "getStreamSampleRate = " << adc.getStreamSampleRate() << '\n' << std::endl;
  
  while ( adc.isStreamRunning() ) {
        usleep(1000.0);
  }
	
	
cleanup:
    if ( adc.isStreamOpen() ) adc.closeStream();
    
    freqmod_destroy(mod);
    
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
   
    return 0;
}

int input( void * /*outputBuffer*/, void *inputBuffer, unsigned int nBufferFrames,
           double /*streamTime*/, RtAudioStreamStatus /*status*/, void *datain )
{
	struct Info *info=(struct Info *)datain;
  	unsigned int frames = nBufferFrames;
  	if(!info->loop)return 2;
  	  	
  	SendData(info,frames,(short *)inputBuffer);
  	
    return 0;
}

void sigIntHandler(const int)
{
    info.loop = 0;
}

int SendData(struct Info *info,unsigned int frames,short *bufin)
{
			unsigned int num,num2;

			for(unsigned int k=0;k<frames;++k){
				buf[k]=0.00002*bufin[k];
			}
			    
	    	msresamp_rrrf_execute(info->iqSampler, (float *)buf, frames, (float *)buf2, &num);  // decimate
		   
		   	freqmod_modulate_block(info->mod, buf2, num, (liquid_float_complex *)r2);

		    		    
		    msresamp_crcf_execute(info->iqSampler2, (liquid_float_complex *)r2, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
			
			std::vector<void *> buffs(2);
			
			int flags(0);


			buffs[0] = buf2;
						
       		int ret = info->device->writeStream(info->txStream,  &buffs[0], num2, flags);
       		if (ret != num2){
            		cout << "error: samples sent: " << ret << "/" << num2 << endl;
            }
            		
            return 0;
}

int printinfo()
{
  RtAudio adc;
  if ( adc.getDeviceCount() < 1 ) {
    std::cout << "\nNo audio devices found!\n";
    return 1;
  }

  int deviceCount=adc.getDeviceCount();
  
  int audiodevice=adc.getDefaultInputDevice();;
  
		printf("\nAudio device Count %d default output device %d audiodevice %d\n",deviceCount,adc.getDefaultOutputDevice(),audiodevice);
	
		RtAudio::DeviceInfo info;
		for (int i=0; i<deviceCount; i++) {
		
			try {
				info=adc.getDeviceInfo(i);
				if(info.outputChannels > 0){
				// Print, for example, the maximum number of output channels for each device
					printf("audio device = %d : output  channels = %d Device Name = %s",i,info.outputChannels,info.name.c_str());
					if(info.sampleRates.size()){
						printf(" sampleRates = ");
						for (int ii = 0; ii < info.sampleRates.size(); ++ii){
							printf(" %d ",info.sampleRates[ii]);
					   }
					}
					printf("\n");
				 }
			 
				if(info.inputChannels > 0){
				// Print, for example, the maximum number of output channels for each device
					printf("audio device = %d : input   channels = %d Device Name = %s",i,info.inputChannels,info.name.c_str());
					 if(info.sampleRates.size()){
						printf(" sampleRates = ");
						for (int ii = 0; ii < info.sampleRates.size(); ++ii){
							printf(" %d ",info.sampleRates[ii]);
					   }
					}
					printf("\n");
			   }

			}
			catch (RtAudioError &error) {
				error.printMessage();
				break;
			}
		
		}
	
		printf("\n");
		
		return 0;
}

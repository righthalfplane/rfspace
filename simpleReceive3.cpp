#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <stdio.h> //printf
#include <stdlib.h> //free
#include <complex.h>
#include <signal.h>

// c++ -std=c++11 -o simpleReceive3 simpleReceive3.cpp -lSoapySDR


int loop = 1;
void sigIntHandler(const int)
{
    loop = 0;
}

int main(void)
{
	size_t length;
		
	signal(SIGINT, sigIntHandler);

    std::string argStr;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    printf("Number of Devices Found: %ld\n",(long)results.size());
    
    if(results.size() < 1)return 1;

    SoapySDR::Kwargs deviceArgs;

   for(unsigned int k=0;k<results.size();++k){
    		printf("SDR device =  %ld ",(long)k);
			deviceArgs = results[k];
			for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
				if (it->first == "label")printf(" %s = %s\n ",it->first.c_str(), it->second.c_str());
			}
    }
    
    printf("\n");

	SoapySDR::Device *sdr = SoapySDR::Device::make(results[3]);

	if (sdr == NULL)
	{
		printf("SoapySDRDevice_make fail: \n");
		return EXIT_FAILURE;
	}
	
	sdr->setSampleRate(SOAPY_SDR_RX, 0, 2e6);
			
	sdr->setFrequency(SOAPY_SDR_RX, 0, 80.3e6);
	
	const std::vector<size_t> channels = {(size_t)0,(size_t)1};
			
	//const std::vector<size_t> channels = {(size_t)0};

	
	SoapySDR::Stream *rxStream = sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, channels);

	sdr->activateStream(rxStream, 0, 0, 0); 


	fprintf(stderr, "start\n");

	//create a re-usable buffer for rx samples
	float buff[400000*2];
	float buff1[400000*2];
 
	//receive some samples
	while (loop)
	{
		void *buffs[] = { buff,buff1 }; //array of buffers
		float *out;
		float *out1;
		int flags=0; //flags set by receive operation
		long long timeNs=0; //timestamp for receive buffer
		int rec=4096;
		out=buff;
		out1=buff1;
		while(rec > 0 && loop){
		    buffs[0]=out;
		    buffs[1]=out1;
		    flags=0;
		    timeNs=0;
			int ret = sdr->readStream(rxStream, buffs, rec, flags, timeNs, 1000000L);
			printf("ret=%d, flags=%d, timeNs=%lld rec %d\n", ret, flags, timeNs, rec);
			//if(ret < 0)fprintf(stderr,"Read Error\n");
			out += 2*(long long)ret;
			out1 += 2*(long long)ret;
			rec -= ret;

		}

	}
	
	    sdr->deactivateStream(rxStream, 0, 0);
    
        sdr->closeStream(rxStream);
    
        SoapySDR::Device::unmake(sdr);


	printf("Done\n");
	return EXIT_SUCCESS;
}
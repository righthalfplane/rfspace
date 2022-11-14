#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <complex.h>
#include <mutex>
#include <liquid/liquid.h>
#include <signal.h>
#include <string.h>
#include <rtaudio/RtAudio.h>
#include <sys/timeb.h>
// c++ -std=c++11 -o nbfmReceive nbfmReceive.cpp -lSoapySDR -lliquid -lrtaudio  -Wall -Wno-return-type-c-linkage
// nbfmReceive "driver=bladerf" 
// nbfmReceive "driver=sdrplay" 
// nbfmReceive "driver=hackrf" 

class cStack *cDUM;

struct rxStruct{
	double fc;
	double f;
	double dt;
	double sino;
	double coso;
	double w;
	double sindt;
	double cosdt;
	double samplerate;
	long size;
	double faudio;
	double bw;
	int mode;
	double amHistory;
	
	class cStack *cs;
	
	int audioOut;
	
	
	double dummy;
	int last;
	msresamp_crcf iqSampler;
	msresamp_rrrf iqSampler2;
	freqdem demod;
	std::mutex mutexo;
	std::mutex mutex1;
	std::mutex mutexa;
	nco_crcf fShift;


};

struct rxStruct rxT;

struct rxStruct *rx=&rxT;

#define NUM_ABUFF 10

#define NUM_DATA_BUFF 10


class cStack{
public:
    cStack(struct rxStruct *rx);
	~cStack();
	int pushBuffa(int nbuffer,struct rxStruct *rx);
	int popBuffa(struct rxStruct *rx);
	int setBuff(struct rxStruct *rx);
	struct rxStruct *rx;    

    float *buffa[NUM_ABUFF];
    int buffStacka[NUM_ABUFF];

    int bufftopa;
    int bufftop;

};

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData );

int loop = 1;
void sigIntHandler(const int)
{
    loop = 0;
}
void sigIntHandler2(const int)
{
    fprintf(stderr,"Error : segfault\n\n");
    
    exit(1);
}

int main(int argc, char** argv)
{

	signal(SIGSEGV, sigIntHandler2);
	
	signal(SIGINT, sigIntHandler);
	
	memset(rx,0,(unsigned char *)&rxT.last-(unsigned char *)&rxT);

	rx->cs=new cStack(rx);

	RtAudio dac;
	
	int deviceCount=dac.getDeviceCount();
		
	if (deviceCount  < 1 ) {
		fprintf(stderr,"\nNo audio devices found!\n");
		return 1;
	}
	
	fprintf(stderr,"RTAUDIO_VERSION %s deviceCount %d default output device %d\n",RTAUDIO_VERSION,deviceCount,dac.getDefaultOutputDevice());
	
    RtAudio::DeviceInfo info;
    for (int i=0; i<deviceCount; i++) {
        
        try {
            info=dac.getDeviceInfo(i);
            if(info.outputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",i,info.outputChannels,info.name.c_str());
             }
             
            if(info.inputChannels > 0){
            // Print, for example, the maximum number of output channels for each device
                fprintf(stderr,"device = %d : maximum output  channels = %d Device Name = %s\n",i,info.inputChannels,info.name.c_str());
            }

        }
        catch (RtAudioError &error) {
            error.printMessage();
            break;
        }
        
    }

	RtAudio::StreamParameters parameters;
	
	int device=1;
	
	if(device == -2){
	    parameters.deviceId = dac.getDefaultOutputDevice();
	}else{
	    parameters.deviceId = device;
	}
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	unsigned int sampleRate = 48000;
	unsigned int bufferFrames = 2400;
	
	try {
		dac.openStream( &parameters, NULL, RTAUDIO_FLOAT32,
						sampleRate, &bufferFrames, &sound, (void *)rx);
		dac.startStream();
	}
	catch ( RtAudioError& e ) {
		e.printMessage();
		exit( 0 );
	}
    	
	rx->fc=200.6e6;
	
	//rx->fc=162.0e6;
	
	rx->faudio=48000;
	
	rx->samplerate=2e6;
	
	//rx->samplerate=912000;
	
	rx->size=rx->samplerate/20;
	
	rx->f=200.3e6;
		
	//rx->f=162.4e6;
	
	rx->cs->setBuff(rx);
	
	rx->audioOut=0;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    fprintf(stderr,"Number of Devices Found: %ld\n",(long)results.size());
    
    if(results.size() < 1)return 1;

    SoapySDR::Kwargs deviceArgs;

   for(unsigned int k=0;k<results.size();++k){
		fprintf(stderr,"SDR device =  %ld ",(long)k);
		deviceArgs = results[k];
		for (SoapySDR::Kwargs::const_iterator it = deviceArgs.begin(); it != deviceArgs.end(); ++it) {
			if (it->first == "label")fprintf(stderr," %s = %s\n",it->first.c_str(), it->second.c_str());
		}
    }
    
    fprintf(stderr,"\n");
    
    std::string argStr(argv[1]);
    
    SoapySDR::Device *sdr = SoapySDR::Device::make(argStr);
    if (sdr == NULL)
    {
        fprintf(stderr,"No device! Found\n");
        return EXIT_FAILURE;
    }
	
	//sdr->setFrequency(SOAPY_SDR_RX, 0, rx->fc);
	
	sdr->setFrequency(SOAPY_SDR_RX,0,"RF",rx->fc);
	
	sdr->setSampleRate(SOAPY_SDR_RX, 0, rx->samplerate);

	//sdr->setGain(SOAPY_SDR_RX, 0, 55.0); 

	SoapySDR::Stream *rxStream = sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, (const std::vector<size_t>)0);

	sdr->activateStream(rxStream, 0, 0, 0); 
	
    float As = 60.0f;
 	
    rx->bw=12500.0;
    
	float Ratio1 = (float)(rx->bw/rx->samplerate);
    
   	float Ratio2=(float)(rx->faudio/rx->bw);

	fprintf(stderr,"Ratio1 %f Ratio %f\n",Ratio1,Ratio2);

    rx->iqSampler  = msresamp_crcf_create(Ratio1, As);
    
    rx->iqSampler2 = msresamp_rrrf_create(Ratio2, As);
    
    rx->demod=freqdem_create(0.5);

    rx->amHistory=0;
        
    float shift=rx->fc-rx->f;
       
    rx->fShift = nco_crcf_create(LIQUID_NCO);
    
    nco_crcf_set_frequency(rx->fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) rx->samplerate))));
   
	//create a re-usable buffer for rx samples
	float *buff1= new float[rx->size*8];
	float *buff2= new float[rx->size*8];

	while (loop)
	{	
		void *buffs[1];
		float *out;
		int flags=0;
		long long timeNs=0;
		int rec=rx->size;
		out=buff1;
		while(rec > 0){
		    buffs[0]=out;
			int ret = sdr->readStream(rxStream, buffs, rec, flags, timeNs, 100000L);
			//printf("ret=%d, flags=%d, timeNs=%lld\n", ret, flags, timeNs);
			if(ret < 0)fprintf(stderr,"Read Error\n");
			out += 2*(long long)ret;
			rec -= ret;
		}
		
		if (shift >= 0) {
             nco_crcf_mix_block_up(rx->fShift, (liquid_float_complex *)buff1, (liquid_float_complex *)buff2, rx->size);
         } else {
             nco_crcf_mix_block_down(rx->fShift, (liquid_float_complex *)buff1, (liquid_float_complex *)buff2,rx->size);
         }
         
		unsigned int num;
		unsigned int num2;
	
		num=0;
		num2=0;

		msresamp_crcf_execute(rx->iqSampler, (liquid_float_complex *)buff2, rx->size, (liquid_float_complex *)buff1, &num);  // decimate
 	
		freqdem_demodulate_block(rx->demod, (liquid_float_complex *)buff1, (int)num, (float *)buff2);

        msresamp_rrrf_execute(rx->iqSampler2, (float *)buff2, num, (float *)buff1, &num2);  // interpolate
		
        //fprintf(stderr,"size %ld num %ld num2 %ld bufferFrames %ld\n",(long)rx->size,(long)num,(long)num2,(long)bufferFrames);
 		
 		int audioOut;

		rx->mutexo.lock();
		audioOut=rx->audioOut;
		float *data=rx->cs->buffa[rx->audioOut++ % NUM_ABUFF];
		rx->mutexo.unlock();

 		for(unsigned int kk=0;kk<num2;++kk){
 		    data[kk]=buff1[kk];
		}
      
      	rx->cs->pushBuffa(audioOut,rx); 		
 		
        //printf("num2 %ld num %ld\n",(long)num2,(long)num);
      
	}
	
	try {
    	// Stop the stream
    	dac.stopStream();
  	}
  	catch (RtAudioError& e) {
    	e.printMessage();
  	}
  	
  	if ( dac.isStreamOpen() ) dac.closeStream();
	
	sdr->deactivateStream(rxStream, 0, 0);

	sdr->closeStream(rxStream);

	SoapySDR::Device::unmake(sdr);
	
	freqdem_destroy(rx->demod);

	msresamp_rrrf_destroy(rx->iqSampler2);
    
    msresamp_crcf_destroy(rx->iqSampler);

    if(buff1)delete[] buff1;
    
    if(buff2)delete [] buff2;
    
    if(rx->cs)delete rx->cs;
    
	fprintf(stderr,"Done\n");
	
	return EXIT_SUCCESS;
}

int sound( void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double streamTime, RtAudioStreamStatus status, void *userData )
{
	struct rxStruct *rx=(struct rxStruct *)userData;
	
	  if ( status )fprintf(stderr,"Stream underflow detected!");
	  
  	float *buffer = (float *) outputBuffer;
	  
	int ibuff=-1;
	if(rx->cs){
		ibuff=rx->cs->popBuffa(rx);
	}
	if (ibuff > 0){
		float *buff= rx->cs->buffa[ibuff % NUM_ABUFF];
	
		int n=0;
	
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			float v=buff[i];
			buffer[n++] = v;
		}
		
	}else{
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			  *buffer++ = 0;
		}
	}
	  
	return 0;
}

cStack::cStack(struct rxStruct *rxi)
{
    rx=rxi;
    bufftopa=0;
    bufftop=0;
    
    for(int k=0;k<NUM_ABUFF;++k){
    	buffStacka[k]=0;
    	buffa[k]=NULL;
    }
}
cStack::~cStack()
{
	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=NULL;
	}
}

int cStack::setBuff(struct rxStruct *rx)
{

	
	for(int k=0;k<NUM_ABUFF;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=(float *)malloc((size_t)(2*rx->faudio*4*sizeof(float)));
		if(!buffa[k]){
			fprintf(stderr,"10 cMalloc Errror %ld\n",(long)(2*rx->faudio*4));
			return 1;
		}
		memset(buffa[k],0,2*rx->faudio*4*sizeof(float));
		buffStacka[k]=-1;
	}


	return 0;
}

int cStack::pushBuffa(int nbuffer,struct rxStruct *rx)
{

	rx->mutexa.lock();
	
    if(bufftopa >= NUM_ABUFF){
        bufftopa=NUM_ABUFF;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	buffStacka[ks]=nbuffer;
        }
   }else{
    	buffStacka[bufftopa++]=nbuffer;
    }
    
	rx->mutexa.unlock();

	return 0;
}

int cStack::popBuffa(struct rxStruct *rx)
{
	int ret;
	
	
	rx->mutexa.lock();
	
	ret=-1;
	
 	if(bufftopa < 1)goto Out;
 	
 	if(bufftopa == 1){
 		ret=buffStacka[0];
 		bufftopa=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftopa;++k){
             if(buffStacka[k] < small2){
             	small2=buffStacka[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	ret=buffStacka[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftopa;++k)
        	{
        		if(k == ks)continue;
        		buffStacka[kk++]=buffStacka[k];
        	}
        	bufftopa--;
        }
	
	
Out:
	rx->mutexa.unlock();

	return ret;
}

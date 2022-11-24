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
// c++ -std=c++11 -o universalReceive universalReceive.cpp -lSoapySDR -lliquid -lrtaudio  -Wall -Wno-return-type-c-linkage
// universalReceive "driver=bladerf" 
// universalReceive "driver=sdrplay" 
// universalReceive "driver=hackrf" 
// universalReceive "driver=lime" 


#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6


struct Filters{
	int np;
    ampmodem demodAM;
	freqdem demod;
	msresamp_crcf iqSampler;
	msresamp_rrrf iqSampler2;
	nco_crcf fShift;
	int thread;	
	double amHistory;
};


class cStack *cDUM;

struct rxStruct{
	double fc;
	double f;
	double samplerate;
	long size;
	double faudio;
	double bw;
	
	class cStack *cs;
	
	int audioOut;
	
	int decodemode;
	
	struct Filters fs;
	
	double gain;
	
	int iprint;
	
	double dummy;
	int last;
	std::mutex mutexo;
	std::mutex mutex1;
	std::mutex mutexa;

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
         
         
int setFilters(struct rxStruct *rx,struct Filters *f);

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

int Usage()
{
	fprintf(stderr,"Usage:\n");
	fprintf(stderr," universalReceive \"driver=hackrf\" [options]\n");
	fprintf(stderr," universalReceive -h\n");
	fprintf(stderr," control-c to stop program\n\n");
	fprintf(stderr,"Mode:\n");
	fprintf(stderr,"  -am               Select AM mode\n");
	fprintf(stderr,"  -nam              Select narrow band AM mode\n");
	fprintf(stderr,"  -fm               Select FM mode\n");
	fprintf(stderr,"  -nbfm             Select narrow band FM mode\n");
	fprintf(stderr,"  -usb              Select upper side band\n");
	fprintf(stderr,"  -lsb              Select lower side band\n");
	fprintf(stderr,"  -cw               Select CW mode\n");
	fprintf(stderr,"\nAdjustments:\n");
	fprintf(stderr,"  -print 1                  Turn on debug print output\n");
	fprintf(stderr,"  -f   162.4                Set radio frequency to  162.4 MHZ\n");
	fprintf(stderr,"  -fc  162.2                Set radio frequency center frequency to  162.2 MHZ\n");
	fprintf(stderr,"  -gain 0.5                 Set gain level to  0.5\n");
	fprintf(stderr,"  -samplerate 2e6           Set sample rate to  2e6\n");
	fprintf(stderr,"  -audiodevice 1            Set select output audio device 1 \n");
	fprintf(stderr,"\nPath:\n");
	fprintf(stderr,"SOAPY_SDR_ROOT\n");

	return 0;
}

int main(int argc, char** argv)
{

	signal(SIGSEGV, sigIntHandler2);
	
	signal(SIGINT, sigIntHandler);
	
	memset((void *)rx,0,(unsigned char *)&rxT.last-(unsigned char *)&rxT);
	
	struct Filters *f=&rx->fs;

	rx->cs=new cStack(rx);
	
	rx->decodemode=MODE_NBFM;
	
	rx->gain=0.0;
	
	rx->fc=200.6e6;
	
	rx->f=200.3e6;
	
	rx->samplerate=2e6;
	
	int device=-2;

	
	if(argc < 3){
        Usage();
        exit(0); 	
 	}


	for(int n=2;n<argc;++n){
	    if(!strcmp(argv[n],"-print")){
		   rx->iprint=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-am")){
		   rx->decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-nam")){
		   rx->decodemode = MODE_NAM;
	    }else if(!strcmp(argv[n],"-fm")){
		   rx->decodemode = MODE_FM;
	    }else if(!strcmp(argv[n],"-nbfm")){
		   rx->decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            rx->decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            rx->decodemode = MODE_LSB;
        }else if(!strcmp(argv[n],"-cw")){
            rx->decodemode = MODE_CW;
	    }else if(!strcmp(argv[n],"-gain")){
	         rx->gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-audiodevice")){
	         device=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         rx->fc=atof(argv[++n]);
	         rx->fc *= 1e6;
	    }else if(!strcmp(argv[n],"-f")){
	         rx->f=atof(argv[++n]);
	         rx->f *= 1e6;
	    }else if(!strcmp(argv[n],"-samplerate")){
            rx->samplerate=atof(argv[++n]);
        }else if(!strcmp(argv[n],"-h")){
            Usage();
            exit(0);
	    }else{
	    	printf("Unknown Command = \"%s\"\n",argv[n]);
	    	Usage();
	    	exit(1);
			// infilename = argv [n] ;
		}
	}
	
	if(fabs(rx->f-rx->fc) > 0.5*rx->samplerate){
		rx->fc=rx->f+0.25*rx->samplerate;
	}
	

	RtAudio dac;
	
	int deviceCount=dac.getDeviceCount();
		
	if (deviceCount  < 1 ) {
		fprintf(stderr,"\nNo audio devices found!\n");
		return 1;
	}
	
	fprintf(stderr,"RTAUDIO_VERSION %s deviceCount %d default output audiodevice %d\n",RTAUDIO_VERSION,deviceCount,dac.getDefaultOutputDevice());
	
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
    	
	
	//rx->fc=162.0e6;
	
	rx->faudio=48000;
	
	
	//rx->samplerate=912000;
	
	rx->size=rx->samplerate/20;
	
		
	//rx->f=162.4e6;
	
	rx->cs->setBuff(rx);
	
	rx->audioOut=0;
        
    std::vector<SoapySDR::Kwargs> results;
    
    results = SoapySDR::Device::enumerate();
    
    fprintf(stderr,"Number of SDR Devices Found: %ld\n",(long)results.size());
    
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
	

	setFilters(rx,f);

    float shift=rx->fc-rx->f;

    nco_crcf_set_frequency(f->fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) rx->samplerate))));
   
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
             nco_crcf_mix_block_up(f->fShift, (liquid_float_complex *)buff1, (liquid_float_complex *)buff2, rx->size);
         } else {
             nco_crcf_mix_block_down(f->fShift, (liquid_float_complex *)buff1, (liquid_float_complex *)buff2,rx->size);
         }
         
		unsigned int num;
		unsigned int num2;
	
		num=0;
		num2=0;

		msresamp_crcf_execute(f->iqSampler, (liquid_float_complex *)buff2, rx->size, (liquid_float_complex *)buff1, &num);  // decimate
		
		if(rx->decodemode < MODE_AM){

			freqdem_demodulate_block(f->demod, (liquid_float_complex *)buff1, (int)num, (float *)buff2);

		}else if(rx->decodemode < MODE_USB){
			
		
			#define DC_ALPHA 0.99    //ALPHA for DC removal filter ~20Hz Fcut with 15625Hz Sample Rate

			for(unsigned int n=0;n<num;++n){
				double mag=sqrt(buff1[2*n]*buff1[2*n]+buff1[2*n+1]*buff1[2*n+1]);
				double z0=mag + (f->amHistory * DC_ALPHA);
				buff2[n]=(float)(z0-f->amHistory);
				f->amHistory=z0;
			}
		/*
		
			ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buff1, (int)num, (float *)buff2);
		*/
		
		}else{
			ampmodem_demodulate_block(f->demodAM,  (liquid_float_complex *)buff1, (int)num, (float *)buff2);
	   }

	   msresamp_rrrf_execute(f->iqSampler2, (float *)buff2, num, (float *)buff1, &num2);  // interpolate
		
        //fprintf(stderr,"size %ld num %ld num2 %ld bufferFrames %ld\n",(long)rx->size,(long)num,(long)num2,(long)bufferFrames);
 		
 		int audioOut;

		rx->mutexo.lock();
		audioOut=rx->audioOut;
		float *data=rx->cs->buffa[rx->audioOut++ % NUM_ABUFF];
		rx->mutexo.unlock();
		
		if(rx->gain == 0.0){
			double amaxs=-1e33;
			for(unsigned int n=0;n<num2;++n)
			{
				double v=fabs(buff1[n]);
				if(v > amaxs)amaxs=v;
			}
			
			if(amaxs <= 0.0)amaxs=1.0;
			
			amaxs=0.95/amaxs;
			
			if(rx->iprint == 1)fprintf(stderr,"1 amax %f gain %f audioOut %d\n",amaxs,rx->gain,audioOut);

			for(unsigned int n=0;n<num2;++n)
			{
			    data[n]=amaxs*buff1[n];
			}
		
		}else{

			for(unsigned int kk=0;kk<num2;++kk){
				data[kk]=rx->gain*buff1[kk];
			}
		
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
	
	freqdem_destroy(f->demod);

	msresamp_rrrf_destroy(f->iqSampler2);
    
    msresamp_crcf_destroy(f->iqSampler);

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
	
		double amax=-1e33;
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			float v=buff[i];
			buffer[n++] = v;
			if(v > amax)amax=v;
		}
		if(rx->iprint > 0)printf("amax %f ibuff %d\n",amax,ibuff);
	}else{
		for (unsigned int i=0; i<nBufferFrames; i++ ) {
			  *buffer++ = 0;
		}
	}
	  
	return 0;
}

int setFilters(struct rxStruct *rx,struct Filters *f)
{
    
    if(!rx)return 1;
    
    float As = 60.0f;
    
    liquid_ampmodem_type mode=LIQUID_AMPMODEM_DSB;
    
    int iflag=0;
    
    if(rx->decodemode == MODE_AM){
        rx->bw=10000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NAM){
        rx->bw=5000.0;
        mode=LIQUID_AMPMODEM_DSB;
        iflag=0;
    } else if(rx->decodemode == MODE_NBFM){
        rx->bw=12500.0;
    }else if(rx->decodemode == MODE_FM){
        rx->bw=200000.0;
    }else if(rx->decodemode == MODE_USB){   // Above 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_USB;
        iflag=1;
    }else if(rx->decodemode == MODE_LSB){  // Below 10 MHZ
        rx->bw=6000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }else if(rx->decodemode == MODE_CW){
        rx->bw=3000.0;
        mode=LIQUID_AMPMODEM_LSB;
        iflag=1;
    }
    
    double Ratio1 = (float)(rx->bw/ rx->samplerate);
    
    double Ratio2=(float)(rx->faudio / rx->bw);

    fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
    
    f->demod=freqdem_create(0.5);
    
#ifdef LIQUID_VERSION_4
    f->demodAM = ampmodem_create(0.5, 0.0, mode, iflag);
 #else
    f->demodAM = ampmodem_create(0.5, mode, iflag);
#endif

    f->iqSampler  = msresamp_crcf_create(Ratio1, As);
    
    f->iqSampler2 = msresamp_rrrf_create(Ratio2, As);    
    
    f->fShift = nco_crcf_create(LIQUID_NCO);
    
    f->amHistory=0;
    
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

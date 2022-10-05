#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#include <string.h>

#include <sndfile.h>

#include <liquid/liquid.h>

#include <signal.h>

#define	BLOCK_SIZE2 200000

#define	BLOCK_SIZE 2048

float buf [BLOCK_SIZE2],r[2*BLOCK_SIZE2];
float buf2 [BLOCK_SIZE2],r2[2*BLOCK_SIZE2];

// c++ -std=c++11 -o simpleTXNBFM simpleTXNBFM.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// simpleTXNBFM "driver=hackrf"
// simpleTXNBFM "driver=bladerf"

int mix(float *buf1,float *buf2,class Listen *l);
int Sleep2(int ms);

int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}

int error()
{
    exit(-1);
}

int main(int argc, char** argv)
{

	char 		infilename[]="/Users/dir/images/saveVoice.wav";
	SNDFILE	 	*infile = NULL ;
	SF_INFO	 	sfinfo ;
	long int count=0;
	int readcount;
	
    const double frequency = 462.7125e6;  //center frequency to 500 MHz
    const double sample_rate = 2e6;    //sample rate to 5 MHz
    float As = 60.0f;
    
    float *buf2=(float *)malloc(2*sample_rate);
	
    std::string argStr(argv[1]);
    
	
		
    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        printf("No device! Found\n");
        return EXIT_FAILURE;
    }

   device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
    
    printf("Sample rate: %g MHZ\n",sample_rate/1e6);

    //Set center frequency
    
     device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     device->setGain(SOAPY_SDR_TX, 0, 25.0);
     
   //  device->setAntenna(SOAPY_SDR_TX, 0, BAND1);
     
     //Streaming Setup
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    
   // size_t MTU=device->getStreamMTU(txStream);
   // printf("MTU: %ld\n",MTU);


    // device->setHardwareTime(0); 

   
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);

	freqmod mod = freqmod_create(0.5); 

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	
		printf ("Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
	} ;
	
	printf ( "Read from file %s.\n", infilename) ;
	printf ( "Number of Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;
		
	
    signal(SIGINT, sigIntHandler);
        	
	float Ratio1 = (float)(12500/ (float)sfinfo.samplerate);
	
	float Ratio2 = (float)(sample_rate/(float)12500);
		
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	std::vector<void *> buffs(2);
	
    int flags(0);

    loop = 1;
	while(loop){
		if ((readcount = sf_readf_float (infile, buf, BLOCK_SIZE)) > 0){
		
			unsigned int num,num2;
			
			float *buffp;
			    
	    	msresamp_rrrf_execute(iqSampler, (float *)buf, readcount, (float *)buf2, &num);  // decimate
		    
		    freqmod_modulate_block(mod, buf2, num, (liquid_float_complex *)r2);
		    
		    //memcpy(r2, buf2,num*8);
		    
		    msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)r2, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
//			fprintf(stderr,"frames %d num %d num2 %d readcount %d\n",BLOCK_SIZE,num,num2,readcount);
			
			
			int numcount=num2;
			
			buffp=buf2;
			
			while(numcount > 0){
				buffs[0] = buffp;
		
				int ret = device->writeStream(txStream,  &buffs[0], numcount, flags);
				if (ret <= 0){
					 printf("writeStream Error ret %d\n",ret);
				}
			
				numcount -= ret;
				buffp += 2*ret;

				count++;
			}
		}else{
		     sf_seek(infile,(sf_count_t)0,SEEK_SET);
		}
	}
	
		
	fprintf(stderr,"count %ld\n",count);

	sf_close (infile) ;
	
    freqmod_destroy(mod);
    
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    return 0;
}




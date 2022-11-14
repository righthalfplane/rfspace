#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#include <string.h>

#include <sndfile.h>

#include <liquid/liquid.h>

#include <signal.h>

#define	BLOCK_SIZE 2048

// c++ -std=c++11 -o nbfmSend nbfmSend.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// nbfmSend "driver=hackrf" "/Users/dir/images/saveVoice.wav"
// nbfmSend "driver=bladerf" "/Users/dir/images/saveVoice.wav"
// nbfmSend "driver=bladerf" "/home/dir/Desktop/saveAudio.wav"

int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}

int main(int argc, char** argv)
{

	char 		*infilename;
	SNDFILE	 	*infile = NULL ;
	SF_INFO	 	sfinfo ;
	
    const double frequency = 200.3e6; 
    const double sample_rate = 2e6;
    const double freqNBFM = 12500;
    float As = 60.0f;
    	
    std::string argStr(argv[1]);
    
    infilename=argv[2];
    
    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        fprintf(stderr,"No device! Found\n");
        return EXIT_FAILURE;
    }

   device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
    
    fprintf(stderr,"Sample rate: %g MHZ\n",sample_rate/1e6);
    
     device->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     device->setGain(SOAPY_SDR_TX, 0, 30.0);
     
    // device->setGain(SOAPY_SDR_TX, 0, 6.0);
    
     //device->setGain(SOAPY_SDR_TX, 0, -20.0);
         
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
   // size_t MTU=device->getStreamMTU(txStream);
   // printf("MTU: %ld\n",MTU);

	if ((infile = sf_open (infilename, SFM_READ, &sfinfo)) == NULL)
	{	
		fprintf(stderr,"Not able to open input file %s.\n", infilename) ;
		puts (sf_strerror (NULL)) ;
		return 1 ;
	} ;
	
	fprintf(stderr, "Read from file %s.\n", infilename) ;
	fprintf(stderr, "Number of Channels %d, Sample rate %d\n", sfinfo.channels, sfinfo.samplerate) ;
		
    signal(SIGINT, sigIntHandler);
        	
	freqmod mod = freqmod_create(0.5); 
	
	float Ratio1 = (float)(freqNBFM/ (float)sfinfo.samplerate);
	
	float Ratio2 = (float)(sample_rate/(float)freqNBFM);
		
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	std::vector<void *> buffs(2);
    long int count=0;
	int readcount;
    int flags(0);
    
    float *buf1 = new float[(int)(BLOCK_SIZE*sizeof(float)*2*Ratio2)];
    
    float *buf2 = new float[(int)(BLOCK_SIZE*sizeof(float)*2*Ratio2)];

    loop = 1;
	while(loop){
		if ((readcount = sf_readf_float (infile, buf1, BLOCK_SIZE)) > 0){
		
			unsigned int num,num2;
			
			float *buffp;
			    
	    	msresamp_rrrf_execute(iqSampler, (float *)buf1, readcount, (float *)buf2, &num);  // decimate
		    
		    freqmod_modulate_block(mod, buf2, num, (liquid_float_complex *)buf1);
		    
		    msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)buf1, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
//			fprintf(stderr,"frames %d num %d num2 %d readcount %d\n",BLOCK_SIZE,num,num2,readcount);

			double amaxs=-1e33;
			for(unsigned int n=0;n<num2*2;++n)
			{
				double v=fabs(buf2[n]);
				if(v > amaxs)amaxs=v;
			}
				
			if(amaxs <= 0.0)amaxs=1.0;
			
			//printf("amax %f\n",amaxs);
			
			for(unsigned int n=0;n<num2*2;++n)
			{
			    buf2[n] = buf2[n]*0.95/amaxs;
			}
				
			int numcount=num2;
			
			buffp=buf2;
			
			while(numcount > 0){
				buffs[0] = buffp;
		
				int ret = device->writeStream(txStream,  &buffs[0], numcount, flags);
				if (ret <= 0){
					 fprintf(stderr,"writeStream Error ret %d\n",ret);
				}
			
				numcount -= ret;
				buffp += 2*ret;
			}
			count++;
		}else{
		     sf_seek(infile,(sf_count_t)0,SEEK_SET);
		}
	}
	
		
	fprintf(stderr,"Blocks Sent %ld\n",count);

	sf_close (infile) ;
	
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    freqmod_destroy(mod);
    
    msresamp_rrrf_destroy(iqSampler);
    
    msresamp_crcf_destroy(iqSampler2);
    
    if(buf1)delete[] buf1;
    
    if(buf2)delete [] buf2;
    
    return 0;
}




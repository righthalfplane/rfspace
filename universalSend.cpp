#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#include <string.h>

#include <sndfile.h>

#include <liquid/liquid.h>

#include <signal.h>

#define MODE_FM   0
#define MODE_NBFM 1
#define MODE_AM   2
#define MODE_NAM  3
#define MODE_USB  4
#define MODE_LSB  5
#define MODE_CW   6

char *names[]={
	(char *)"fm",
	(char *)"nbfm",
	(char *)"am",
	(char *)"nam",
	(char *)"usb",
	(char *)"lsb",
	(char *)"cm",
};

//#define LIQUID_VERSION_4 1


#define	BLOCK_SIZE 2048

// c++ -std=c++11 -o universalSend universalSend.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage
// universalSend "driver=hackrf" "/Users/dir/images/saveVoice.wav"
// universalSend "driver=bladerf" "/Users/dir/images/saveVoice.wav"
// universalSend "driver=lime" "/home/dir/Desktop/saveAudio.wav"
// universalSend "driver=bladerf" "/home/dir/Desktop/saveAudio.wav"

int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}
int Usage()
{
	fprintf(stderr,"Usage:\n");
	fprintf(stderr," universalSend \"driver=hackrf\" \"/home/dir/Desktop/saveAudio.wav\" [options]\n");
	fprintf(stderr," universalSend -h\n");
	fprintf(stderr," control-c to stop program\n\n");
	fprintf(stderr,"Mode:\n");
	fprintf(stderr,"  -am               Select AM mode\n");
	fprintf(stderr,"  -nam              Select narrow band AM mode\n");
	fprintf(stderr,"  -nbfm             Select narrow band FM mode\n");
	fprintf(stderr,"  -usb              Select upper side band\n");
	fprintf(stderr,"  -lsb              Select lower side band\n");
	fprintf(stderr,"\nAdjustments:\n");
	fprintf(stderr,"  -print 1                  Turn on debug print output\n");
	fprintf(stderr,"  -f   162.4                Set radio frequency to  162.4 MHZ\n");
	fprintf(stderr,"  -fc  162.2                Set radio frequency center frequency to  162.2 MHZ\n");
	fprintf(stderr,"  -gain 0.5                 Set gain level to  0.5\n");
	fprintf(stderr,"  -rf_gain 30               Set gain level to  30\n");
	fprintf(stderr,"\nPath:\n");
	fprintf(stderr,"SOAPY_SDR_ROOT\n");

	return 0;
}

int main(int argc, char** argv)
{
	char 		*infilename;
	SNDFILE	 	*infile = NULL ;
	SF_INFO	 	sfinfo ;
	
    double sample_rate = 2e6;
    float As = 60.0f;
 	int decodemode=MODE_NBFM;
 	int iprint=0;
 	double rf_gain=30.0;
 	double gain=0.0;
 	double fc=200.1e6;
 	double f=200.3e6;
 	
 	if(argc < 3){
        Usage();
        exit(0); 	
 	}
 	
	for(int n=3;n<argc;++n){
	   // fprintf(stderr,"%d %s\n",n,argv[n]);
	    if(!strcmp(argv[n],"-print")){
		   iprint=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-am")){
		   decodemode = MODE_AM;
	    }else if(!strcmp(argv[n],"-nam")){
		   decodemode = MODE_NAM;
	    }else if(!strcmp(argv[n],"-nbfm")){
			decodemode = MODE_NBFM;
        }else if(!strcmp(argv[n],"-usb")){
            decodemode = MODE_USB;
        }else if(!strcmp(argv[n],"-lsb")){
            decodemode = MODE_LSB;
	    }else if(!strcmp(argv[n],"-gain")){
	         gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-rf_gain")){
	         rf_gain=atof(argv[++n]);
	    }else if(!strcmp(argv[n],"-fc")){
	         fc=atof(argv[++n]);
	         fc *= 1e6;
	    }else if(!strcmp(argv[n],"-f")){
	         f=atof(argv[++n]);
	         f *= 1e6;
        }else if(!strcmp(argv[n],"-h")){
            Usage();
            exit(0);
	    }else if(!strcmp(argv[n],"-samplerate")){
            sample_rate=atof(argv[++n]);
	    }else{
	    	printf("Unknown Command = \"%s\"\n",argv[n]);
			// infilename = argv [n] ;
            Usage();
            exit(0);
		}
	}
 	
 	if(fabs(f-fc) > 0.5*sample_rate){
		fc=f+0.25*sample_rate;
	}
	
 	
 	 double offset=f-fc;

    std::string argStr(argv[1]);
    
    infilename=argv[2];
    
    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        fprintf(stderr,"No device! Found\n");
        return EXIT_FAILURE;
    }

   	device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
    
    fprintf(stderr,"Sample rate: %g MHZ Mode: %s\n",sample_rate/1e6,names[decodemode]);
    
     device->setFrequency(SOAPY_SDR_TX, 0, "RF", f);
    
     device->setGain(SOAPY_SDR_TX, 0, rf_gain);
              
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    if(!txStream){
        fprintf(stderr,"txStream Error %p\n",txStream);
        exit(1);
    }
    
    
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
    
    
    unsigned int samples=sfinfo.samplerate;
        	
	freqmod mod = freqmod_create(0.5); 
	
	float Ratio1 = (float)(10000/ (float)samples);
	
	float Ratio2 = (float)(sample_rate/(float)10000);
	

    liquid_ampmodem_type type=LIQUID_AMPMODEM_DSB;
    int flag=0;
    if(decodemode == MODE_NBFM){
        Ratio1 = (float)(12500/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)12500);
    }else if(decodemode == MODE_USB){
        type=LIQUID_AMPMODEM_USB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }else if(decodemode == MODE_LSB){
        type=LIQUID_AMPMODEM_LSB;
        Ratio1 = (float)(6000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)6000);
        flag=1;
    }else if(decodemode == MODE_NAM){
        type=LIQUID_AMPMODEM_DSB;
        Ratio1 = (float)(5000/ (float)samples);
        Ratio2 = (float)(sample_rate/(float)5000);
        flag=0;
    }
	
    ampmodem demodAM;
    
#ifdef LIQUID_VERSION_4
    demodAM = ampmodem_create(0.5, 0.0, type, flag);
#else
    demodAM = ampmodem_create(0.5, type, flag);
#endif

	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	msresamp_rrrf iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	msresamp_crcf iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	nco_crcf oscillator = nco_crcf_create(LIQUID_NCO);
	
	nco_crcf_set_phase(oscillator, 0);
	
	double pi=4.0*atan(1.0);
	
  	nco_crcf_set_frequency(oscillator, 2*pi*offset/sample_rate);

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
		    		    
			if(decodemode == MODE_AM || decodemode == MODE_NAM){
				ampmodem_modulate_block(demodAM,buf2, num, (liquid_float_complex *)buf1);
			}else if(decodemode == MODE_NBFM || decodemode == MODE_FM){
				freqmod_modulate_block(mod, buf2, num, (liquid_float_complex *)buf1);
			}else{
				ampmodem_modulate_block(demodAM,buf2, num, (liquid_float_complex *)buf1);
			}
		    
		    msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)buf1, num, (liquid_float_complex *)buf2, &num2);  // decimate
			
//			fprintf(stderr,"frames %d num %d num2 %d readcount %d\n",BLOCK_SIZE,num,num2,readcount);

         	//if(offset != 0)nco_crcf_mix_block_up(oscillator, (liquid_float_complex *)buf2, (liquid_float_complex *)buf1, num2);

			if(iprint == 1){
				double amaxs=-1e33;
				for(unsigned int n=0;n<num2*2;++n)
				{
					double v=fabs(buf2[n]);
					if(v > amaxs)amaxs=v;
				}
				
				if(amaxs <= 0.0)amaxs=1.0;
			
				fprintf(stderr,"amax %f\n",amaxs);
			}
			
			if(gain == 0.0){

				double amaxs=-1e33;
				for(unsigned int n=0;n<num2*2;++n)
				{
					double v=fabs(buf2[n]);
					if(v > amaxs)amaxs=v;
				}
			
				if(amaxs <= 0.0)amaxs=1.0;
				
				amaxs=0.95/amaxs;
			
				if(iprint == 1)fprintf(stderr,"1 amax %f gain %f\n",amaxs,gain);

				for(unsigned int n=0;n<num2*2;++n)
				{
					buf2[n]=amaxs*buf2[n];
				}

			}else{

				for(unsigned int n=0;n<num2*2;++n)
				{
					buf2[n] = gain*(buf2[n]);
				}
			
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
    
    if(demodAM)ampmodem_destroy(demodAM);
    
    nco_crcf_destroy(oscillator);
    
    if(buf1)delete[] buf1;
    
    if(buf2)delete [] buf2;
    
    return 0;
}




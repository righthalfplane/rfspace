#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#include <string.h>

#include <sndfile.h>

#include <liquid/liquid.h>

#include <unistd.h>

#include "Poly.h"

#define	BLOCK_SIZE 2048


// c++ -std=c++11 -o simpleTXTwo simpleTXTwo.cpp Poly.cpp Utilities.cpp -lSoapySDR -lsndfile -lliquid -Wall -Wno-return-type-c-linkage -Wno-deprecated-declarations
// simpleTXTwo "driver=hackrf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/saveAudio.wav"
// simpleTXTwo "driver=bladerf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/saveAudio.wav"
// simpleTXTwo "driver=bladerf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/eye.wav"

class Send{
public:
	Send(double frequency,double samplerate,char *filename,double offset);
	int transScribe(float *buf1,float *buf2,unsigned int *num22);
	int mix(float *buf1,float *buf2,int size);
	~Send();
	char *infilename;
	SNDFILE	 	*infile;
	SF_INFO	 	sfinfo;
	msresamp_rrrf iqSampler;
	msresamp_crcf iqSampler2;
	ampmodem demodAM;
	nco_crcf oscillator;
	double samplerate;
	double offset;
	double sino,coso,sindt,cosdt,dt;
	Poly *p;

};

int Send::mix(float *buf1,float *buf2,int size)
{

	   double sint,cost;
    
    for (int k = 0 ; k < size ; k++){
        float r = buf1[k * 2];
        float i = buf1[k * 2 + 1];
        if(dt > 0){
            buf2[k * 2] = (float)(r*coso - i*sino);
            buf2[k * 2 + 1] = (float)(i*coso + r*sino);
            sint=sino*cosdt+coso*sindt;
            cost=coso*cosdt-sino*sindt;
            coso=cost;
            sino=sint;
        }else{
            buf2[k * 2] = r;
            buf2[k * 2 + 1] = i;
        }
    }
    
    double rr=sqrt(coso*coso+sino*sino);
    coso /= rr;
    sino /= rr;
    
	return 0;

}

Send::Send(double frequency,double sampleratei,char *filename,double offseti)
{
	samplerate=sampleratei;
	offset=offseti;
	if ((infile = sf_open (filename, SFM_READ, &sfinfo)) == NULL)
	{	
		printf ("Not able to open input file %s.\n", filename) ;
		puts (sf_strerror (NULL)) ;
		return;
	}
	 
	float As = 60.0f;
	    
	float Ratio1 = (float)(10000/ (float)sfinfo.samplerate);
	
	float Ratio2 = (float)(samplerate/(float)10000);
		
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_DSB, 0);
	
	oscillator = nco_crcf_create(LIQUID_NCO);
	
	nco_crcf_set_phase(oscillator, 0);

	if(offset != 0.0){
		double pi=4.0*atan(1.0);
		nco_crcf_set_phase(oscillator, 0);
  		nco_crcf_set_frequency(oscillator, 2*pi*offset/samplerate);
  		
		dt=1.0/(double)samplerate;
		sino=0;
		coso=1;
		double w=2.0*pi*(offset);
		sindt=sin(w*dt);
		cosdt=cos(w*dt);
 		
	}
	
    	p=new Poly((float)samplerate);
        p->Cbandpass("butter",16,1.0,0.5*samplerate+offset-5000,0.5*samplerate+offset+5000);
        p->forceCascadeStart();
}

int Send::transScribe(float *buf1,float *buf2,unsigned int *num22)
{
	int readcount;

	*num22=0;
	
	if ((readcount = sf_readf_float (infile, buf1, BLOCK_SIZE)) > 0){
	
		unsigned int num,num2;
			
		msresamp_rrrf_execute(iqSampler, (float *)buf1, readcount, (float *)buf2, &num);	

		ampmodem_modulate_block(demodAM, buf2, num, (liquid_float_complex *)buf1);
/*
	    float dmin = 1e33;
        float dmax =-1e33;
        for(int i=0;i<num;++i){
            float v=buf1[i];
            if(v < dmin)dmin=v;
            if(v > dmax)dmax=v;
        }
        
      fprintf(stderr,"b dmin %f dmin %f\n",dmin,dmax);
*/

	
		msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)buf1, num, (liquid_float_complex *)buf2, &num2);
		
		if(offset != 0.0){
			nco_crcf_mix_block_up(oscillator, (liquid_float_complex *)buf2, (liquid_float_complex *)buf2, num2);
		    //mix(buf2,buf2,num2);
		   // p->forceCascadeRun(buf2,buf2,num2,0);
		}

		*num22=num2;
	}else{
		 sf_seek(infile,(sf_count_t)0,SEEK_SET);		
	}

	return 0;
}
Send::~Send()
{


    if(iqSampler)msresamp_rrrf_destroy(iqSampler);
    
    if(iqSampler2)msresamp_crcf_destroy(iqSampler2);
    
    if(demodAM)ampmodem_destroy(demodAM);
    
    if(oscillator)nco_crcf_destroy(oscillator);

	sf_close (infile) ;
	printf("~Send\n");
}

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

	long int count=0;
	
    const double frequency = 205.5e6;
    const double sample_rate = 2e6;
    	
    std::string argStr(argv[1]);
    
    SoapySDR::Device *device = SoapySDR::Device::make(argStr);
    if (device == NULL)
    {
        printf("No device! Found\n");
        return EXIT_FAILURE;
    }

   device->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);
    
    printf("Sample rate: %g MHZ\n",sample_rate/1e6);

     device->setFrequency(SOAPY_SDR_TX, 0, "RF", frequency);
    
     device->setGain(SOAPY_SDR_TX, 0, 30.0);
     
    std::vector<size_t> channels;

	channels = {0};

    SoapySDR::Stream *txStream = device->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    if(!txStream){
        fprintf(stderr,"txStream Error %p\n",txStream);
        exit(1);
    }
   
    int ret4=device->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
	
	Send *s1 = new Send(frequency,sample_rate,argv[2],0.0);
	
	Send *s2 = new Send(frequency,sample_rate,argv[3],100000);
		
    signal(SIGINT, sigIntHandler);
        		
	std::vector<void *> buffs(2);
	
    int flags(0);

    float *buf1 = new float[(int)(sizeof(float)*2*sample_rate)];
    
    float *buf2 = new float[(int)(sizeof(float)*2*sample_rate)];

    float *buf3 = new float[(int)(sizeof(float)*2*sample_rate)];

    loop = 1;
	while(loop){
		
		
			unsigned int num2;
			
			    
			s1->transScribe(buf1,buf2,&num2);
			if(!num2)continue;
			
		
			s2->transScribe(buf1,buf3,&num2);
			if(!num2)continue;
			//fprintf(stderr,"2 num2 %d\n",num2);


            double gain=0.0;
			
			if(gain == 0.0){

				double amaxs=-1e33;
				for(unsigned int n=0;n<num2*2;++n)
				{
					double v=fabs(buf2[n])+fabs(buf3[n]);
					if(v > amaxs)amaxs=v;
				}
			
				if(amaxs <= 0.0)amaxs=1.0;
				
				amaxs=0.95/amaxs;
			
				//fprintf(stderr,"1 amax %f gain %f\n",amaxs,gain);

				for(unsigned int n=0;n<num2*2;++n)
				{
					buf2[n]=amaxs*(buf2[n]+buf3[n]);
				}
			}else{

				for(unsigned int n=0;n<num2*2;++n)
				{
					buf2[n] = gain*(buf2[n]+buf3[n]);
				}
			
			}
			
	
			int numcount=num2;
			
			float *buffp;
		
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
	
	}
		
	fprintf(stderr,"count %ld\n",count);
	
    device->deactivateStream(txStream);

    device->closeStream(txStream);
    
    SoapySDR::Device::unmake(device);
    
    if(buf1)delete[] buf1;
    
    if(buf2)delete [] buf2;
    
    if(buf3)delete [] buf3;
    
    if(s1)delete s1;
    
    if(s2)delete s2;

    
    return 0;
}




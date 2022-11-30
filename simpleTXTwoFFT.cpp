#include <stdio.h>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>

#include <string.h>

#include <sndfile.h>

#include <liquid/liquid.h>

#include <unistd.h>

#include <fftw3.h>

#include <signal.h>

#define	BLOCK_SIZE 2048

// c++ -std=c++11 -o simpleTXTwoFFT simpleTXTwoFFT.cpp -lSoapySDR -lsndfile -lliquid  -lfftw3 -lfftw3f -Wall -Wno-return-type-c-linkage
// simpleTXTwoFFT "driver=hackrf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/saveAudio.wav"
// simpleTXTwoFFT "driver=bladerf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/saveAudio.wav"
// simpleTXTwoFFT "driver=bladerf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/eye.wav"
// simpleTXTwoFFT "driver=hackrf" "/Users/dir/images/saveVoice.wav" "/Users/dir/images/eye.wav"

//int fftSize=16384;
int fftSize=10000;

class Send{
public:
	Send(double frequency,double samplerate,char *filename,double offset);
	int transScribe(float *buf1,float *buf2,unsigned int *num22);
	int getFile(float *buf1,float *buf2,unsigned int *num22);
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
	freqmod mod;
};


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
	    
	float Ratio1 = (float)(fftSize/ (float)sfinfo.samplerate);
	
	//float Ratio1 = (float)(8192/ (float)32768);
	
	float Ratio2 = (float)(samplerate/(float)fftSize);
		
	fprintf(stderr,"Ratio1 %g Ratio2 %g\n",Ratio1,Ratio2);
	
	iqSampler  =  msresamp_rrrf_create(Ratio1, As);
	
	iqSampler2  = msresamp_crcf_create(Ratio2, As);
	
	demodAM = ampmodem_create(0.5, LIQUID_AMPMODEM_DSB, 0);
	
	oscillator = nco_crcf_create(LIQUID_NCO);
	
	nco_crcf_set_phase(oscillator, 0);
	
	mod = freqmod_create(0.5);

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
	
}
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

int Send::transScribe(float *buf1,float *buf2,unsigned int *num22)
{
	int readcount;

	*num22=0;
	
	if ((readcount = sf_readf_float (infile, buf1, BLOCK_SIZE)) > 0){
	
	
		fprintf(stderr,"readcount %d BLOCK_SIZE %d\n",readcount,BLOCK_SIZE);
			
		unsigned int num,num2;
			
		msresamp_rrrf_execute(iqSampler, (float *)buf1, readcount, (float *)buf2, &num);	

		ampmodem_modulate_block(demodAM, buf2, num, (liquid_float_complex *)buf1);
	
		msresamp_crcf_execute(iqSampler2, (liquid_float_complex *)buf1, num, (liquid_float_complex *)buf2, &num2);
		
		fprintf(stderr,"num %d BLOCK_SIZE %d\n",num,num2);

		
		if(offset != 0.0){
			//nco_crcf_mix_block_up(oscillator, (liquid_float_complex *)buf2, (liquid_float_complex *)buf2, num2);
		    //mix(buf2,buf2,num2);
		    //forceCascadeRun(p,buf2,buf2,num2,0);
		}

		*num22=num2;
	}else{
		 sf_seek(infile,(sf_count_t)0,SEEK_SET);		
	}

	return 0;
}

int Send::getFile(float *buf1,float *buf2,unsigned int *num22)
{
	int readcount;

	*num22=0;
	
	if ((readcount = sf_readf_float (infile, buf2, sfinfo.samplerate)) > 0){
				
		unsigned int num;
		
		if(readcount < sfinfo.samplerate){
		     sf_seek(infile,(sf_count_t)0,SEEK_SET);		
			 int read = sf_readf_float (infile, &buf2[readcount], sfinfo.samplerate-readcount);
			 if(read+readcount != sfinfo.samplerate){
			 	fprintf(stderr,"sf_readf_float Error\n");
			 }
			 readcount=sfinfo.samplerate;
		
		}
			
		msresamp_rrrf_execute(iqSampler, (float *)buf2, readcount, (float *)buf1, &num);
		
		ampmodem_modulate_block(demodAM, buf1, num, (liquid_float_complex *)buf2);
	
		*num22=num;
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
}


int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}

int main(int argc, char** argv)
{

	long int count=0;
	
    const double frequency = 200.0e6;
    //const double sample_rate = 2.0e6;
    const double sample_rate = 2.097152e6;
    int samplerate=(int)sample_rate;
        	
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
    
     device->setGain(SOAPY_SDR_TX, 0, 50.0);
          
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
	
	Send *s2 = new Send(frequency,sample_rate,argv[3],-100000);
		
    signal(SIGINT, sigIntHandler);
        		
	std::vector<void *> buffs(2);
	
    int flags(0);

    float *buf1 = new float[(int)(sizeof(float)*4*sample_rate)];
    
    float *buf2 = new float[(int)(sizeof(float)*4*sample_rate)];

    float *buf3 = new float[(int)(sizeof(float)*4*sample_rate)];

    float *buf4 = new float[(int)(sizeof(float)*4*sample_rate)];
    
	fftwf_plan p1;
				
	p1 = fftwf_plan_dft_1d(samplerate,(fftwf_complex *)buf2, (fftwf_complex *)buf4, FFTW_BACKWARD, FFTW_ESTIMATE);
	
	fftwf_plan p2;
				
	p2 = fftwf_plan_dft_1d(fftSize,(fftwf_complex *)buf3, (fftwf_complex *)buf3, FFTW_FORWARD, FFTW_ESTIMATE);

	fftwf_plan p3;
				
	p3 = fftwf_plan_dft_1d(fftSize,(fftwf_complex *)buf4, (fftwf_complex *)buf4, FFTW_FORWARD, FFTW_ESTIMATE);

    loop = 1;
	while(loop){
		
		
			unsigned int num1,num2;

			for(int n=0;n<fftSize;++n)
			{
				buf3[2*n]=0.0;
				buf3[2*n+1]=0.0;
				buf4[2*n]=0.0;
				buf4[2*n+1]=0.0;
			}

			s1->getFile(buf1,buf3,&num1);
			if(!num1)continue;
			
			fftwf_execute(p2);

			for(int n=0;n<fftSize/2;++n){
			    int n1=fftSize-n-1;
				double r=buf3[2*n1];
				double i=buf3[2*n1+1];
				int n2=fftSize/2-n-1;
				buf3[2*n1]=buf3[2*n2];
				buf3[2*n1+1]=buf3[2*n2+1];
				buf3[2*n2]=r;
				buf3[2*n2+1]=i;
				
			}

			s2->getFile(buf1,buf4,&num2);
			if(!num2)continue;
			
			fftwf_execute(p3);
			

			for(int n=0;n<fftSize/2;++n){
			    int n1=fftSize-n-1;
				double r=buf4[2*n1];
				double i=buf4[2*n1+1];
				int n2=fftSize/2-n-1;
				buf4[2*n1]=buf4[2*n2];
				buf4[2*n1+1]=buf4[2*n2+1];
				buf4[2*n2]=r;
				buf4[2*n2+1]=i;
				
			}
						
			for(int n=0;n<samplerate;++n)
			{
				buf2[2*n]=0;
				buf2[2*n+1]=0;
			}
			
			int ns1=300000-0.5*fftSize;
			int ns2=500000-0.5*fftSize;
			for(int n=0;n<fftSize;++n)
			{
				buf2[2*ns1]=buf3[2*n];
				buf2[2*ns1+1]=buf3[2*n+1];
				ns1++;
				buf2[2*ns2]=buf4[2*n];
				buf2[2*ns2+1]=buf4[2*n+1];
				ns2++;
			}
			
			fftwf_execute(p1);

			double amaxs=-1e33;
			for(int n=0;n<samplerate*2;++n)
			{
				double v=fabs(buf4[n]);
				if(v > amaxs)amaxs=v;
			}
			
			
			for(int n=0;n<samplerate*2;++n)
			{
				 buf4[n]=0.95*(buf4[n])/amaxs;
			}
								
			int numcount=samplerate;
			
			float *buffp;
		
			buffp=buf4;
			
			while(numcount > 0){
				buffs[0] = buffp;
		
				int ret = device->writeStream(txStream,  &buffs[0], numcount, flags);
				if (ret <= 0){
					 printf("writeStream Error ret %d\n",ret);
				}
			
				numcount -= ret;
				buffp += 2*ret;
			}
			count++;
	
	}
		
		
	fftwf_destroy_plan(p1);


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




//
// ofdmflexframesync_example.c - From liquid-sdr
//
// Example demonstrating the OFDM flexible frame synchronizer.
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <csignal>
#include <chrono>
#include <stdarg.h>
#include <thread>


#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Errors.hpp>
#include <SoapySDR/Time.hpp>


#include <liquid/liquid.h>

#include <mutex>

#define NUM_DATA_BUFF 10
#define NUM_DATA_BUFF2 10
#define NUM_ABUFF2     10

extern void *cMalloc(unsigned long r, int tag);
extern int zerol(unsigned char *p,unsigned long n);
extern int cFree(char *p);


//c++ -std=c++11 -o ofdmfile.x ofdmfile.cpp -lliquid -lSoapySDR -Wno-return-type-c-linkage -Wno-deprecated-declarations
//ofdmfile.x -M 32 -n 256 -t -d "driver=lime" junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=hackrf"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=bladerf"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=rtlsdr"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=uhd"  junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -t -d "driver=lime" -f 230 junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=bladerf" -f 230 -o 0.01 junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -t -d "driver=uhd" -f 230 junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=lime" -f 230 junkreceive.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=rtlsdr" -f 230 junkreceive.cpp
//iqSDR.x -f 2400.2 -fc 2400.3 -device 0
//bladeRF-cli -l /Users/dir/Desktop/blade/hostedxA5-latest.rbf
//ofdmfile.x -M 32 -n 256 -t -d "driver=bladerf" -f 230.2 junksend.cpp
//ofdmfile.x -M 32 -n 256 -d "driver=hackrf" -f 230.2 -o 0.01 junkreceive.cpp



class cStack{
public:
    cStack();
	~cStack();
	int popBuff(float *buffOut,int SizeOut);
	int pushBuff(int nbuffer);
	int popBuff(void);
	int pushBuffa(int nbuffer);
	int popBuffa();
	int setBuff(int sizei,int faudioi);
	
	float *buff[NUM_DATA_BUFF2];
    int buffStack[NUM_DATA_BUFF2];
    
    short int *buffa[NUM_ABUFF2];
    int buffStacka[NUM_ABUFF2];

	int size;
	int faudio;

    int bufftopa;
    int bufftop;
    
    std::mutex mutexa;    
    std::mutex mutexo;
    std::mutex mutex1;

};

class rxClass{
public:
    // options
    unsigned int      M               = 64;                 // number of subcarriers
    unsigned int      cp_len          = 16;                 // cyclic prefix length
    unsigned int      taper_len       = 4;                  // taper length
    unsigned int      payload_len     = 120;                // length of payload (bytes)
    modulation_scheme ms              = LIQUID_MODEM_QPSK;  // modulation scheme
    fec_scheme        fec0            = LIQUID_FEC_NONE;    // inner code
    fec_scheme        fec1            = LIQUID_FEC_HAMMING128; // outer code
    crc_scheme        check           = LIQUID_CRC_32;      // validity check
    int               debug           =  0;                 // enable debugging?
    char              *outDevice      = (char *)"driver=lime";
    char              *file           = NULL;
    int               iTransmit       = 0;
    double 			  fc			  = 230;
    double 			  foffset		  = 0;
	ofdmflexframegen  fg              = NULL; 

	SoapySDR::Stream *rxStream;
	
	SoapySDR::Device *device;
	
	volatile int doWhat;
	
	volatile int witch;	
	
	class cStack *bS;
	
	int rxBuffer();
	int readSDR();
    size_t MTU;
	
	int retFlag;
	
	int size; 
	
	int iWait=0;


};
/*
rxClass::rxClass()
{
	fprintf(stderr,"rxClass::rxClass Called\n");
}
rxClass::~rxClass()
{	
	
}
*/
int setRadio(SoapySDR::Device *device);

int send(class rxClass *rx,unsigned int payload_len,ofdmflexframegen fg,char *filein,
      char *outDevice,unsigned int M,unsigned int cp_len,double frequency,double foffset);

int receive(class rxClass *rx,unsigned int payload_len,ofdmflexframegen fg,char *fileout,
      char *outDevice,unsigned int M,unsigned int cp_len,double fcc,double foffset);

int Sleep2(int ms);

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata);
             
int loop = 0;
void sigIntHandler(const int)
{
    loop = 0;
}



void usage()
{
    printf("ofdmflexframesync_example [options]\n");
    printf(" -h        : print usage\n");
    printf(" -s  <snr> : signal-to-noise ratio [dB], default: 20\n");
    printf(" -F <freq> : carrier frequency offset, default: 0.002\n");
    printf(" -M  <num> : number of subcarriers (must be even), default: 64\n");
    printf(" -C  <len> : cyclic prefix length, default: 16\n");
    printf(" -n  <len> : payload length [bytes], default: 120\n");
    printf(" -m  <mod> : modulation scheme (qpsk default)\n");
    liquid_print_modulation_schemes();
    printf(" -v  <crc> : data integrity check: crc32 default\n");
    liquid_print_crc_schemes();
    printf(" -c  <fec> : coding scheme (inner): h74 default\n");
    printf(" -k  <fec> : coding scheme (outer): none default\n");
    liquid_print_fec_schemes();
    printf(" -D       : enable debugging\n");
    printf(" -d  <driver=bladerf>     : output device\n");
}



int main(int argc, char*argv[])
{
    //srand(time(NULL));
    
    class rxClass *rx=new class rxClass;
    


    // get options
    int dopt;
    while((dopt = getopt(argc,argv,"uhDs:F:M:C:n:m:v:c:k:d:tf:o:")) != EOF){
        switch (dopt) {
        case 'u':
        case 'h': usage();                    return 0;
        case 'D': rx->debug       = 1;            break;
        case 'M': rx->M           = atoi(optarg); break;
        case 'C': rx->cp_len      = atoi(optarg); break;
        case 'n': rx->payload_len = atol(optarg); break;
        case 'm': rx->ms          = liquid_getopt_str2mod(optarg); break;
        case 'v': rx->check       = liquid_getopt_str2crc(optarg); break;
        case 'c': rx->fec0        = liquid_getopt_str2fec(optarg); break;
        case 'k': rx->fec1        = liquid_getopt_str2fec(optarg); break;
        case 'd': rx->outDevice   = optarg; break;
        case 't': rx->iTransmit   = 1;      break;
        case 'f': rx->fc          = atof(optarg)*1e6; break;
        case 'o': rx->foffset     = atof(optarg)*1e6; break;
       default:
            exit(-1);
        }
        
    }
    if(optind < argc)
  	{
    	rx->file = argv[optind];
  	} else{
    	rx->file = NULL;
  	}

	fprintf(stderr,"file %s iTransmit %d fc %g foffset %g\n",rx->file,rx->iTransmit,rx->fc,rx->foffset);
	

    // create frame generator
    ofdmflexframegenprops_s fgprops;
    ofdmflexframegenprops_init_default(&fgprops);
    fgprops.check           = rx->check;
    fgprops.fec0            = rx->fec0;
    fgprops.fec1            = rx->fec1;
    fgprops.mod_scheme      = rx->ms;
    ofdmflexframegen fg = ofdmflexframegen_create(rx->M, rx->cp_len, rx->taper_len, NULL, &fgprops);
    
    rx->fg=fg;

	if(rx->iTransmit){
		send(rx,rx->payload_len,rx->fg,rx->file,rx->outDevice,rx->M,rx->cp_len,rx->fc,rx->foffset);
	}else{
		receive(rx,rx->payload_len,rx->fg,rx->file,rx->outDevice,rx->M,rx->cp_len,rx->fc,rx->foffset);		
	}
	
    return 0;
}

// callback function
int callback(unsigned char *  _header,
             int              _header_valid,
             unsigned char *  _payload,
             unsigned int     _payload_len,
             int              _payload_valid,
             framesyncstats_s _stats,
             void *           _userdata)
{
    //fprintf(stderr,"**** callback invoked : rssi = %8.3f dB, evm = %8.3f dB, cfo = %8.5f\n", _stats.rssi, _stats.evm, _stats.cfo);

    unsigned int i;
    
    FILE *out=(FILE *)_userdata;
    
    if(out){
    	int ret1=fwrite(_payload,1,_payload_len,out);
    	if(ret1 < 0)fprintf(stderr,"Error Writing output file\n");
	}
    // print header data to standard output
    long int *ipp=(long int *)(&_header[0]);
    fprintf(stderr,"  header rx  : %ld ",*ipp);
    for (i=0; i<8; i++)
        fprintf(stderr," %d", _header[i]);
    fprintf(stderr,"\n");
/*
    // print payload data to standard output
   fprintf(stderr,"  payload rx :");
    for (i=0; i<_payload_len; i++) {
        fprintf(stderr," %d", _payload[i]);
        if ( ((i+1)%26)==0 && i !=_payload_len-1 )
            fprintf(stderr,"\n              ");
    }
    fprintf(stderr,"\n");
*/
    // count errors in received payload and print to standard output
    //unsigned char * payload_tx = (unsigned char*) _userdata;
    //unsigned int num_errors = count_bit_errors_array(_payload, payload_tx, _payload_len);
   // fprintf(stderr,"  bit errors : %u / %u\n", num_errors, 8*_payload_len);

    return 0;
}
int rxClass::readSDR()
{

   
	float *buff=bS->buff[witch % NUM_DATA_BUFF];
	 
	void *buffs[] = {buff};
	
	int toRead=size;
	
	int ret=0;
	
	int count=0;
		 
	while(doWhat == 2){
		long long timeNs=0;	
		int flags=0;
		
		buffs[0]=buff+2*count;
		
		unsigned int iread;
		
		iread=toRead;
		if(iread > MTU)iread=MTU;
		
		 ret = device->readStream(rxStream, buffs, iread, flags, timeNs, 100000L);
		 
		 retFlag=ret;
	 
		 timeNs++;
				   
		if(ret <= 0){
		   fprintf(stderr,"read Error: \"%s\" ret %d iread %d MTU %ld witch %d\n","windowName",ret,iread,(long)MTU,witch);
		 //  if(Debug > 0)winout("readStream ret %d \n",ret);
		   break;
		}else if(ret < toRead){
			count += ret;
			toRead=toRead-ret;
		}else{
			break;
		}
	}

	return 0;
}

int rxClass::rxBuffer()
{
	class rxClass *rx=this;
	

	while(1)
	{
		if(rx->doWhat < 0){
			rx->doWhat=-2;
			//winout("rxBuffer Exit\n");
			return 0;
		}
	     switch(rx->doWhat){
	     case 0:
	     	Sleep2(50);
			//winout("rxBuffer Sleep\n");
	        break;
	     case 1:
	        return 0;
		 case 2:
			//auto t1 = std::chrono::high_resolution_clock::now();
			
			readSDR();
				
	        if(rx->doWhat == 2){
	        	float *buff=bS->buff[rx->witch % NUM_DATA_BUFF];
/*     	
 	static FILE *out11;
 	if(!out11)out11=fopen("test1_IQ_101500000_10000000_fc.raw","wb");
 	if(out11)fwrite(buff,8,retFlag,out11);
*/       
 	
 	/*
	            if(rx->IQSwap){
                    for(int n=0;n<rx->size;++n){
                        float save;
                        save=buff[2*n];
                        buff[2*n]=buff[2*n+1];
                        buff[2*n+1]=save;
                    }
                }
    */
                int idc=0;
                if(idc){
                	float average=0;
                    for(int n=0;n<rx->size*2;++n){
                    	average += buff[n];
                    }
                    average /= (rx->size*2);
                    
                    for(int n=0;n<rx->size*2;++n){
                    	buff[n] -= average;
                    }
                    
                    fprintf(stderr,"average %g\n",average);
          
                }

	        	bS->pushBuff(rx->witch);
	        	
             	++rx->witch;

				
	        }
	        //auto t2 = std::chrono::high_resolution_clock::now();
		    //std::chrono::duration<double> difference = t2 - t1;
		    //winout("Time %g rx->witch %d\n",difference.count(),rx->witch);
	//		winout("rxBuffer doWhat %p %d\n",&rx->doWhat,rx->doWhat);
	        break;
	     }
	     
	}
	return 0;
}




int receive(class rxClass *rx,unsigned int payload_len,ofdmflexframegen fg,char *fileout,
      char *outDevice,unsigned int M,unsigned int cp_len,double fcc,double foffset)
{

	FILE *out=NULL;
	
	if(fileout){
		out=fopen(fileout,"wb");
		if(!out){
			fprintf(stderr,"Could not Open File %s to write\n",fileout);
			exit(1);		
		}
	}

    // allocate memory for header, payload
    unsigned char header[8];
    unsigned char payload[payload_len];
    
     unsigned int  taper_len=4;
    
    ofdmflexframesync fs = ofdmflexframesync_create(M, cp_len, taper_len, NULL, callback, (void*)out);
   

    // initialize header/payload and assemble frame
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);
    ofdmflexframesync_print(fs);
	unsigned int symbols = ofdmflexframegen_getframelen(fg);
	unsigned int symbolSize = M + cp_len;
	unsigned int frameSize = symbols * symbolSize;
	unsigned int buf_len=frameSize;
	
	float *buf=new float[buf_len*2];
    

    const double fc = fcc+foffset;  //center frequency to 230 MHz
    const double f = fcc;  //center frequency to 230 MHz
    const double sample_rate = 2000000;    //sample rate to 2 MHz
    float As = 60.0f;
    nco_crcf fShift;
    float shift=fc-f;
    fShift = nco_crcf_create(LIQUID_NCO);
    nco_crcf_set_frequency(fShift,(float) ((2.0 * M_PI) * (((double) abs(shift)) / ((double) sample_rate))));

    
     std::string argStr(outDevice);
    
			
    SoapySDR::Device *device2 = SoapySDR::Device::make(argStr);
    if (device2 == NULL)
    {
        fprintf(stderr,"No device!\n");
        return EXIT_FAILURE;
    }
    
    setRadio(device2);

   device2->setSampleRate(SOAPY_SDR_RX, 0, sample_rate);

    fprintf(stderr, "Sample rate: %g MHz\n",sample_rate/1e6);

    //Set center frequency
    
     device2->setFrequency(SOAPY_SDR_RX, 0, fc);
    
     device2->setGain(SOAPY_SDR_RX, 0, 35.0);
     
     //device2->setAntenna(SOAPY_SDR_RX, 0, BAND1);
     
     //Streaming Setup

	const std::vector<size_t> channels = {(size_t)0};

    SoapySDR::Stream *rxStream = device2->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
    
    rx->device=device2;
    
    rx->rxStream=rxStream;
    
    
    int ret4=device2->activateStream(rxStream, 0, 0, 0);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
    
    size_t MTU;
    
    MTU=device2->getStreamMTU(rxStream);
   
	float Ratio1 = (float)(10000.0/(float)sample_rate);
	
	msresamp_crcf iqSampler1  = msresamp_crcf_create(Ratio1, As);

	printf("Ratio1 %g buf_len %d\n",Ratio1,buf_len);
	
    
    float *buf2= new float[(int)(10*sample_rate*8)];
    
    float *buf3= new float[(int)(10*sample_rate*8)];
    
    signal(SIGINT, sigIntHandler);
    
    unsigned int num1;
 
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    
  	//unsigned int bufsize=buf_len/Ratio1;
   
   
    class cStack *bS=new cStack;

    rx->bS=bS;
    
    int size=sample_rate/20.0;
    
    rx->size=size;
    
   	unsigned int bufsize=rx->size;
   
    rx->MTU=MTU;
    
    size += 1024;  // bug in rfspace NetSDR and icr8600
    
    bS->setBuff(size,(int)48000);
    
    rx->doWhat=0;

	rx->witch=0;

    std::thread(&rxClass::rxBuffer,rx).detach();
    
    rx->doWhat=2;
    
    //fprintf(stderr,"rx->m %d\n",rx->M);

    // generate frame, push through channel
    loop = 1;
	while(loop){	
		{			

 			unsigned int tosend=bufsize;

			float *out=buf2;
			
			int ip=bS->popBuff(out,bufsize);
			if(ip < 0){
				Sleep2(5);
				continue;
			}
			

    		static FILE *out11=NULL;
/*
 			if(!out11){
 				char *name=(char *)"Receive_IQ_230000000_2000000_fc.raw";
 				out11=fopen(name,"wb");
 				if(!out11)fprintf(stderr,"open error %s\n",name);
 			}
 			
 			if(out11)fwrite(out,8,bufsize,out11);

*/

		if (shift >= 0) {
             nco_crcf_mix_block_up(fShift, (liquid_float_complex *)buf2, (liquid_float_complex *)buf3, bufsize);
         } else {
             nco_crcf_mix_block_down(fShift, (liquid_float_complex *)buf2, (liquid_float_complex *)buf3,bufsize);
         }
         
         for(int n=0;n<rx->size*2;++n)buf2[n]=buf3[n];

			//for(int n=0;n<buf_len;++n){
 			//	fprintf(stderr,"%d %g %g %p\n",n,buf2[2*n],buf2[2*n+1],out11);
 			//}
			//exit(1);
			     

			//fprintf(stderr,"end sample_rate %ld\n",(long)sample_rate);
			
			//fprintf(stderr,"num1 = %d buf_len = %d buf[0] %g %g\n",bufsize,buf_len,buf2[0],buf2[bufsize-1]);
			
			num1=0;

			msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)buf2, bufsize, (liquid_float_complex *)buf, &num1);  // decimate

			
			//fprintf(stderr,"bufsize = %d size = %d num1 = %d buf_len = %d buff[0] %g %g\n",bufsize,rx->size,num1,buf_len,buf[0],buf[buf_len-1]);


			ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, num1);
		}
    
    }
    
    rx->doWhat = -1;

    // export debugging file

    // destroy objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    
    device2->deactivateStream(rxStream);

    device2->closeStream(rxStream);
    
    SoapySDR::Device::unmake(device2);
    
    

    printf("done.\n");
    return 0;

	return 0;
}
 
int send(class rxClass *rx,unsigned int payload_len,ofdmflexframegen fg,char *filein,
      char *outDevice,unsigned int M,unsigned int cp_len,double frequency,double foffset)
{
	FILE *in=NULL;
	
	if(filein){
		in=fopen(filein,"rb");
		if(!in){
			fprintf(stderr,"Could not Open File %s to read\n",filein);
			exit(1);		
		}
	}

    unsigned int  buf_len = 4096;
    float buf[10*buf_len*2]; // time-domain buffer

    // allocate memory for header, payload
    unsigned char header[8];
    unsigned char payload[payload_len];
    
    unsigned int  taper_len=4;
    
    ofdmflexframesync fs = ofdmflexframesync_create(M, cp_len, taper_len, NULL, callback, (void*)payload);


    // initialize header/payload and assemble frame
    for (int i=0; i<8; i++)
        header[i] = i & 0xff;
    for (int i=0; i<payload_len; i++)
        payload[i] = i & 0xff;
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);
    ofdmflexframesync_print(fs);
        unsigned int symbols = ofdmflexframegen_getframelen(fg);
    	unsigned int symbolSize = M + cp_len;
    	unsigned int frameSize = symbols * symbolSize;
    	buf_len=frameSize;

    // create channel and add impairments    
    //const double frequency = 230.0e6;  //center frequency to 230 MHz
    const double sample_rate = 2000000;    //sample rate to 2 MHz
    float As = 60.0f;
    
     std::string argStr(outDevice);
    
    std::vector<size_t> channels;

	channels = {0};
	
		
    SoapySDR::Device *device2 = SoapySDR::Device::make(argStr);
    if (device2 == NULL)
    {
        fprintf(stderr,"No device!\n");
        return EXIT_FAILURE;
    }

   device2->setSampleRate(SOAPY_SDR_TX, 0, sample_rate);

    fprintf(stderr, "Sample rate: %g MHz\n",sample_rate/1e6);

    //Set center frequency
    
     device2->setFrequency(SOAPY_SDR_TX, 0, frequency);
    
     device2->setGain(SOAPY_SDR_TX, 0, 55.0);
     
   //  device->setAntenna(SOAPY_SDR_TX, 0, BAND1);
     
     //Streaming Setup

    SoapySDR::Stream *txStream = device2->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32, channels);
    //fprintf(stderr,"txStream %p\n",txStream);
   
    int ret4=device2->activateStream(txStream);
    if(ret4)fprintf(stderr,"ret4 %d\n",ret4);
   
	float Ratio1 = (float)(sample_rate/(float)10000.);
	
	msresamp_crcf iqSampler1  = msresamp_crcf_create(Ratio1, As);

	printf("Ratio1 %g buf_len %d\n",Ratio1,buf_len);
	
    
    float *buf2= new float[(int)(4*sample_rate*8)];
    
    signal(SIGINT, sigIntHandler);
    
    unsigned int num1;
    
    long int loopcount=0;
    
    auto t1 = std::chrono::high_resolution_clock::now();

	double amax=0;
   
    loop = 1;
	while(loop){
        //fprintf(stderr,"loop  = %d buf_len %d in %p\n",loop,buf_len,in);
        unsigned int paylen=payload_len;
    	for (int i=0; i<8; i++)
        	header[i] = i & 0xff;
        ++loopcount;
        long int *ipp=(long int *)(&header[0]);
        *ipp=loopcount;
    	for (int i=0; i<paylen; i++)
        	payload[i] = i & 0xff;
        if(in){
           int ret1=fread(payload,1,paylen,in);
           //fprintf(stderr,"ret1 %d\n",ret1);
           if(ret1 <= 0){
	            auto t2 = std::chrono::high_resolution_clock::now();
		        std::chrono::duration<double> difference = t2 - t1;
           		fprintf(stderr,"Rewind Input File length %ld Bytes Seconds %g loopcount %ld\n",(long)ftell(in),difference.count(),loopcount);
           		--loopcount;
           		rewind(in);
           		continue;
           }
           paylen=ret1;
        }
        ofdmflexframegen_assemble(fg, header, payload, paylen);
        //fprintf(stderr,"symbols %d symbolSize %d frameSize %d\n",symbols,symbolSize,frameSize);
        
  

		//unsigned int *ifh=(unsigned int *)fg;
		int last_symbol=0;
		while (!last_symbol) {
			// generate symbol
			last_symbol = ofdmflexframegen_write(fg, (liquid_float_complex *)buf, buf_len);
			//fprintf(stderr,"%d %f %f  %f %f %ld\n",last_symbol,buf[0],buf[1],buf[2*(256-1)],buf[2*(256-1)+1],(long)ifh[16]);
//			ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, buf_len);
			break;
			
		}  
		
		//ofdmflexframegen_reset(fg);

			//fprintf(stderr,"buf_len %d\n",buf_len);
			
			msresamp_crcf_execute(iqSampler1, (liquid_float_complex *)buf, buf_len, (liquid_float_complex *)buf2, &num1);
			
			//fprintf(stderr,"num1 %d\n",num1);

 			unsigned int tosend=num1;

			float *out=buf2;
			
			std::vector<void *> buffs(2);
	
    		int flags(0);
    		
    		for(int n=0;n<tosend;++n){    		
    			double v=sqrt(out[2*n]*out[2*n]+out[2*n+1]*out[2*n+1]); 			
    			if(v > amax)amax=v;
    		}
    		
    		//fprintf(stderr,"amax %g\n",amax);
    		
    		for(int n=0;n<tosend*2;++n){
    			out[n] = out[n]*0.95/amax;
    		}

    		
    		//static FILE *out11;
 			//if(!out11)out11=fopen("Sent_IQ_230000000_2000000_fc.raw","wb");
 			//if(out11)fwrite(out,8,num1,out11);
   		
			while(1){
			    int tomove;
				buffs[0] = out;
				tomove=tosend;
				if(tomove > 25000)tomove=25000;
				
				int ret = device2->writeStream(txStream,  &buffs[0], tomove, flags);
				if(ret < 0){
					fprintf(stderr, "writeStream ret %d\n",ret);
				}
	
				if(ret == (int)tosend)break;
				out += 2*ret;
				tosend -= ret;
				if(tosend <= 0){
					//cout << "error: samples sent: " << tosend << "/" << num2 << endl;
					break;
				}
			

			//ofdmflexframesync_execute(fs, (liquid_float_complex *)buf, buf_len);
		}
		//fprintf(stderr,"num1 %u\n",num1);
    
    }


    // destroy objects
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    
    device2->deactivateStream(txStream);

    device2->closeStream(txStream);
    
    SoapySDR::Device::unmake(device2);
    
    fprintf(stderr,"done.\n");

	return 0;
}
int Sleep2(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	return 0;
}

int setRadio(SoapySDR::Device *device)
{

    std::vector< std::string > str_list;
    
    
	fprintf(stderr,"driver = %s\n",device->getDriverKey().c_str());

	fprintf(stderr,"hardware = %s\n",device->getHardwareKey().c_str());

	SoapySDR::Kwargs it=device->getHardwareInfo();

	for( SoapySDR::Kwargs::iterator  ii=it.begin(); ii!=it.end(); ++ii)
	{
			fprintf(stderr,"%s = %s ",ii->first.c_str(), ii->second.c_str());
	}


	fprintf(stderr,"\n\n");
    
    
	str_list = device->listAntennas( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx antennas: ");
	for(int i = 0; i < str_list.size(); ++i)
		fprintf(stderr,"%s,", str_list[i].c_str());
	fprintf(stderr,"\n");
	
	
	fprintf(stderr,"Setting Info: \n");
			
	SoapySDR::ArgInfoList args = device->getSettingInfo();
	if (args.size()) {
		for (SoapySDR::ArgInfoList::const_iterator args_i = args.begin(); args_i != args.end(); args_i++) {
			SoapySDR::ArgInfo arg = (*args_i);

			fprintf(stderr,"key %s value %s read %s type %d min %g max %g step %g\n",arg.key.c_str(),arg.value.c_str(),device->readSetting(arg.key).c_str(),
				(int)arg.type,arg.range.minimum(),arg.range.maximum(),arg.range.step());

		}
	}
	fprintf(stderr,"\n");
	
	std::vector<std::string> streamFormat=device->getStreamFormats(SOAPY_SDR_RX, 0);

	for (size_t j = 0; j < streamFormat.size(); j++)
	{
		fprintf(stderr,"RX StreamFormats %lu %s\n",j, streamFormat[j].c_str());
		if(j == streamFormat.size()-1)fprintf(stderr,"\n");
	}
	
	str_list = device->listGains( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx Gains: ");
	for(int i = 0; i < str_list.size(); ++i)
		fprintf(stderr,"%s, ", str_list[i].c_str());
	fprintf(stderr,"\n");
    
	SoapySDR::RangeList ranges = device->getFrequencyRange( SOAPY_SDR_RX, 0);
	fprintf(stderr,"Rx freq ranges: ");
	for(int i = 0; i < ranges.size(); ++i)
		fprintf(stderr,"[%g Hz -> %g Hz], ", ranges[i].minimum(), ranges[i].maximum());
	fprintf(stderr,"\n");
	
	std::vector<double> band=device->listBandwidths(SOAPY_SDR_RX, 0);
	if(band.size()){
		fprintf(stderr,"\nBandwidth MHZ ");  		
		for (size_t j = 0; j <band.size(); j++)
		{
		   fprintf(stderr," %.2f ",band[j]/1.0e6);
		}
		fprintf(stderr,"\n\n");
	}
	
	
	std::vector<double> rate=device->listSampleRates(SOAPY_SDR_RX, 0);
	if(rate.size()){
		 fprintf(stderr,"SampleRates MHZ ");
	}
	for (size_t j = 0; j < rate.size(); j++)
	{
		fprintf(stderr," %.6f ",rate[j]/1.0e6);
	}
	fprintf(stderr,"\n\n");
	
	int hasGainMode=device->hasGainMode(SOAPY_SDR_RX, 0);
	
	if(hasGainMode){
		bool automatic=true;
		device->setGainMode(SOAPY_SDR_RX,0, automatic);
		fprintf(stderr,"getGainMode: %d\n",device->getGainMode(SOAPY_SDR_RX, 0));
	}

	int hasFrequencyCorrection= device->hasFrequencyCorrection(SOAPY_SDR_RX, 0);

	fprintf(stderr,"hasFrequencyCorrection: %d\n",hasFrequencyCorrection);

	
	
	

	return 0;
}


cStack::cStack()
{
    bufftopa=0;
    bufftop=0;
    
    for(int k=0;k<NUM_DATA_BUFF2;++k){
        buffStack[k]=0;
        buff[k]=NULL;
    //	winout("k %d buff[k] %p\n",k,buff[k]);
    }
    for(int k=0;k<NUM_ABUFF2;++k){
    	buffStacka[k]=0;
    	buffa[k]=NULL;
    }
}
cStack::~cStack()
{
	for(int k=0;k<NUM_DATA_BUFF2;++k){
		if(buff[k])free((char *)buff[k]);
		buff[k]=NULL;
	}
	
	for(int k=0;k<NUM_ABUFF2;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=NULL;
	}
	
	//winout("Exit cStack %p\n",this);
}

int cStack::setBuff(int sizei,int faudioi)
{
	size=sizei;
	faudio=faudioi;

	//fprintf(stderr,"setBuff size %d faudio %d\n",sizei,faudio);

	for(int k=0;k<NUM_DATA_BUFF2;++k){
	//    winout("k %d buff %p\n",k,buff[k]);
		if(buff[k])free((char *)buff[k]);
		buff[k]=(float *)malloc(2*size*4*8);
		if(!buff[k]){
			fprintf(stderr,"5 cMalloc Errror %ld\n",(long)(2*size*4));
			return 1;
		}
		memset((unsigned char *)buff[k],0,2*size*4);
		buffStack[k]=-1;
	}
	
	for(int k=0;k<NUM_ABUFF2;++k){
		if(buffa[k])free((char *)buffa[k]);
		buffa[k]=(short int *)malloc((size_t)(2*faudio*4));
		if(!buffa[k]){
			fprintf(stderr,"10 cMalloc Errror %ld\n",(long)(2*faudio*4));
			return 1;
		}
		memset((unsigned char *)buffa[k],0,(unsigned long)(2*faudio*4));
		buffStacka[k]=-1;
	}


	return 0;
}

int cStack::pushBuffa(int nbuffer)
{

	mutexa.lock();
//	mprint("pushBuffa in %d\n",bufftopa);
	
    if(bufftopa >= NUM_ABUFF2){
        bufftopa=NUM_ABUFF2;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_ABUFF2;++k){
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
    
//    mprint("pushBuffa nbuffer %d top %d\n",nbuffer,rx->bufftopa);
    
//	mprint("pushBuffa out\n");
	mutexa.unlock();

	return 0;
}

int cStack::popBuffa()
{
	int ret;
	
	
	mutexa.lock();
//	mprint("popBuffa in %d\n",bufftopa);
	
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
//    if(ret > 0)mprint("popBuffa ret %d top %d\n",ret,rx->bufftopa);
//	mprint("popBuffa out\n");
	mutexa.unlock();

	return ret;
}

int cStack::pushBuff(int nbuffer)
{

	mutex1.lock();
	
	//winout("pushBuff nbuffer %d buffStack %p\n",nbuffer,buffStack);

	
    if(bufftop >= NUM_DATA_BUFF2){
        bufftop=NUM_DATA_BUFF2;
        int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<NUM_DATA_BUFF2;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
        
        if(ks >= 0){
        	buffStack[ks]=nbuffer;
        }
   }else{
    	buffStack[bufftop++]=nbuffer;
    }
    
	mutex1.unlock();

	
	return 0;
}
int cStack::popBuff()
{
	int ret;
	
	
	mutex1.lock();

	
	ret=-1;
	
	
 	if(bufftop < 1)goto Out;
 	
  	//winout("popBuff bufftop %d \n",bufftop );
	
 	if(bufftop == 1){
 		ret=buffStack[0];
 		bufftop=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftop;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
  	//winout("ks %d \n",ks);
       
        if(ks >= 0){
        	ret=buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftop;++k)
        	{
        		if(k == ks)continue;
        		buffStack[kk++]=buffStack[k];
        	}
        	bufftop--;
        }
	
	
Out:
	mutex1.unlock();

	return ret;
}
int cStack::popBuff(float *buffOut,int SizeOut)
{
	int ret;
	
	
	mutex1.lock();

	
	ret=-1;
	
	
 	if(bufftop < 1)goto Out;
 	
  	//winout("popBuff bufftop %d \n",bufftop );
	
 	if(bufftop == 1){
 		ret=buffStack[0];
 		bufftop=0;
 		goto Out;
 	}
 	
       int small2,ks;
        small2=1000000000;
        ks=-1;
        for(int k=0;k<bufftop;++k){
             if(buffStack[k] < small2){
             	small2=buffStack[k];
             	ks=k;
             }
        }
  	//winout("ks %d \n",ks);
       
        if(ks >= 0){
        	ret=buffStack[ks];
        	int kk;
        	kk=0;
        	for(int k=0;k<bufftop;++k)
        	{
        		if(k == ks)continue;
        		buffStack[kk++]=buffStack[k];
        	}
        	bufftop--;
        }
	
Out:
	if(ret >= 0){
	
	    int witch=ret % NUM_DATA_BUFF2;
		float *buffIn=buff[witch];
		for(int n=0;n<SizeOut*2;++n){
			buffOut[n]=buffIn[n];
		}
	}
	mutex1.unlock();

	return ret;
}

